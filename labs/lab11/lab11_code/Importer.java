/*  Written by Ariel Rabkin <asrabkin@gmail.com>, 2011.
 * Copyright 2011, the Regents of the University of California.
  
  Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

import java.io.*;
import java.security.MessageDigest;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.SequenceFile;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.compress.*;
import org.apache.hadoop.io.compress.bzip2.CBZip2InputStream;

/**
 * Converts text files to sequence files, suitably for cs61c project 1, Spring 201
 *
 * Usage: Importer <file or directory> [output]
 * If invoked on a text file, converts that file to compressed sequence file, writing the output
 * in output dir.
 * If invoked on a directory, recursively scans that directory and subdirs for .txt
 * files, storing output to output dir.
 * 
 * Each input file is split at boundaries, where a boundary is a line containing
 * exactly the text: "---END.OF.DOCUMENT---"
 * 
 * Will also process .bz2 files, first decompressing them.
 * 
 * Default output dir is "convertedOut"
 *
 *  Written by Ariel Rabkin, asrabkin@gmail.com
 *  Licensed under the terms of the New BSD License. 
 *  
 */
public class Importer {

  static SequenceFile.Writer seqFileWriter;
  static long totalBytes = 0;
  static long totalRecords = 0;
  static long files = 0;
  static File outDir = new File("convertedOut");
  public static void main(String[] args) {
    try {
      if(args.length < 1) {
        System.err.println("can't run. Not enough args. Need to specify input file or dir");
        System.exit(-1);
      } else
        System.out.println("starting scan of " + args[0]);
      
      if(args.length > 1)
        outDir = new File(args[1]);
      System.out.println("dumping output to " + outDir.getAbsolutePath());
      
      lookForFiles(new File(args[0]));
      long avgRecLength = totalBytes / totalRecords;
      System.out.println("total data, uncompressed was " + totalBytes/ (1024 * 1024) + " MB");
      System.out.println("total of " + totalRecords + " records. (Avg uncompressed size " + avgRecLength + " bytes)");
    } catch(Exception e) {
      e.printStackTrace();
    }
  }
  
  public static Text hash(Text content) throws Exception {
    StringBuilder sb = new StringBuilder();
    sb.append("doc_");

    MessageDigest md = MessageDigest.getInstance("MD5");

    md.update(content.getBytes(), 0, content.getLength());
    byte[] bytes = md.digest();
    for(int i=0; i < bytes.length; ++i) {
      if( (bytes[i] & 0xF0) == 0)
        sb.append('0');
      sb.append( Integer.toHexString(0xFF & bytes[i]) );
    }
    return new Text(sb.toString());
  }
  
  static void lookForFiles(File file) throws Exception {
    if(file.isDirectory()) {
      File[] contents = file.listFiles();
      if(contents == null) {
        System.out.println("WARN: null list of contents for " + file.getAbsolutePath());
        return;
      }
      for(File sub: contents)
        lookForFiles(sub);
    } else {
      if(file.getName().endsWith(".bz2") || file.getName().contains(".txt"))
        copyFile(file);
    }
  }
  
  public static void copyFile(File file) throws Exception {
//    String TEST_PREFIX = "";
    File destFile = new File(outDir,file.getName()+".seq");
    Path dest = new Path(destFile.getAbsolutePath());
    
    Configuration conf = new Configuration();
    FileSystem fileSys = org.apache.hadoop.fs.FileSystem.get(new java.net.URI(conf.get("fs.default.name")),conf);
    CompressionCodec codec = new DefaultCodec();
    fileSys.mkdirs(dest.getParent());
    FSDataOutputStream outputStr = fileSys.create(dest);
    seqFileWriter = SequenceFile.createWriter(conf, outputStr,
        Text.class, Text.class,
        SequenceFile.CompressionType.BLOCK, codec);
    String filename = file.getName();
    InputStream in = new BufferedInputStream(new FileInputStream(file));
    if(filename.endsWith(".bz2")) {
     in.read();
     in.read(); //snarf header
     in = new CBZip2InputStream(in);
    }
    BufferedReader br = new BufferedReader(new InputStreamReader(in, "US-ASCII"));
    
    System.out.println("working on file " + file);
    int records = 0;
    long bytes = 0, bytes_since_status = 0;
    long startTime= System.currentTimeMillis();
    String s = null;
    Text content = new Text();
    while( (s = br.readLine()) != null) {
      if(s.startsWith("---END.OF.DOCUMENT---")) {
        Text name = new Text(hash(content));
        seqFileWriter.append(name, content);
        records ++;
        content = new Text();
      } else {
        byte[] line_as_bytes = (s+ " ").getBytes();
        for(byte b: line_as_bytes) {
          assert b < 128: "found an unexpected high-bit set";
        }

        content.append(line_as_bytes, 0, line_as_bytes.length);
        bytes += line_as_bytes.length;
        /*
        bytes_since_status += line_as_bytes.length;
        if(bytes_since_status > 10 * 1024 * 1024) { //every 10 MB
          System.err.print('.');
          bytes_since_status = 0;
        }*/
      }
    } //end while
    if(content.getLength() > 5) {
      Text name = new Text(hash(content));
      seqFileWriter.append(name, content);
      records ++;
    }
    totalBytes += bytes;
    totalRecords += records;
    long time = (System.currentTimeMillis() - startTime)/ 1000 + 1;
    long kbSec = bytes / 1024 / time;
    System.out.println(new java.util.Date());
    System.out.println("File " + file.getName() + " " + records+ " records, " + 
        bytes + " bytes in " + time+ " seconds ("  +kbSec + " KB/sec).");
    in.close();
    seqFileWriter.close();
    outputStr.close();
  }
}
