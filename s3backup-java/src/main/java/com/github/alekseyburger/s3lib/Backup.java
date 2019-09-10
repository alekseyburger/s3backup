package com.github.alekseyburger.s3lib;

import com.amazonaws.ClientConfiguration;
import com.amazonaws.Protocol;
import com.amazonaws.auth.AWSCredentials;
import com.amazonaws.auth.BasicAWSCredentials;
import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.AmazonS3Client;
import com.amazonaws.services.s3.model.*;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.List;

public class Backup {

    final static String accessKey = "demo";
    final static String secretKey = "demo";
    final static String sevrer_net = "127.0.0.1:8080";

    Logger logger;

    private AccessControlList acls;
    private CannedAccessControlList cannnedAcls;

    AWSCredentials credentials;
    ClientConfiguration clientConfig;
    AmazonS3 conn;

    public Backup(Logger logger) {

        this.logger = logger;

        this.credentials = new BasicAWSCredentials(accessKey, secretKey);

        this.clientConfig = new ClientConfiguration();
        this.clientConfig.setProtocol(Protocol.HTTP);

        conn = new AmazonS3Client(credentials, clientConfig);
        conn.setEndpoint(this.sevrer_net);

        cannnedAcls = CannedAccessControlList.PublicRead;

        acls = new AccessControlList();
        acls.setOwner(new Owner("demo", "demo"));
        //acls.addGrant(new Grant(UserGroups.ALL_USERS.name(), Permission.READ, GrantType.GROUP))
        acls.grantPermission(GroupGrantee.AllUsers, Permission.FullControl);
        acls.grantPermission(GroupGrantee.AuthenticatedUsers, Permission.Read);
        acls.grantPermission(GroupGrantee.AuthenticatedUsers, Permission.Write);

        logger.info("Start Backup");
    }

    public void buckets_check_create(String bucketName) {
        List<Bucket> buckets = conn.listBuckets();
        for (Bucket bucket : buckets) {
            if (bucket.getName() == bucketName) {
                return;
            }
        }
        try {
            CreateBucketRequest request = new CreateBucketRequest(bucketName);
            request.setCannedAcl(cannnedAcls);
            conn.createBucket(request);
        } catch (Exception exept) {
            System.out.println("Bucket creation error: " + exept.getMessage());
        }
    }

    public void backup_file(String bucketName, String fileName) {

        try {
            java.net.URI fullName = Backup.class.getClassLoader().getResource("object.txt").toURI();

            File file = new File(fullName);


            PutObjectRequest putRequest1 = new PutObjectRequest(bucketName, fileName, file);

            PutObjectResult response1 = conn.putObject(putRequest1);
            conn.setObjectAcl(bucketName, fileName, acls);

        } catch (Exception exept) {
            System.out.println("File creation error: " + exept.getMessage());
            System.out.println(exept.getClass().getName());
        }
    }
}
