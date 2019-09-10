package com.github.alekseyburger.application;

import com.github.alekseyburger.s3lib.Backup;
import org.apache.log4j.Logger;

import java.time.LocalDate;
import java.time.format.DateTimeFormatter;

public class s3backup {

    final static Logger logger = Logger.getLogger(s3backup.class);

    public static void main(String[] args) {

        Backup app = new Backup(logger);
        final String bucketName = "javabucket";

        DateTimeFormatter dtf = DateTimeFormatter.ofPattern("yyyy.MM.dd");
        LocalDate localDate = LocalDate.now();
        String uniqDirName = dtf.format(localDate);

        app.buckets_check_create(bucketName);

        String uniqFileName = uniqDirName + "/object." + System.currentTimeMillis();
        app.backup_file(bucketName, uniqFileName);
    }
}
