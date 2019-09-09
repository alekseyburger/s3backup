#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/Object.h>

#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/logging/DefaultLogSystem.h>
#include <aws/core/utils/logging/AWSLogging.h>

class s3_application {

    const Aws::String  accessKeyId = Aws::String("demo");
    const Aws::String secretKey = Aws::String("demo");
    const Aws::String endpoint = Aws::String("127.0.0.1:8080");
    
    //const std::string bucket_name = "backup";
    Aws::Auth::AWSCredentials*   p_credentials;
    Aws::Client::ClientConfiguration configuration;
    Aws::S3::S3Client* p_s3_client;

    static Aws::SDKOptions options;
    static const char* app_name;
    static unsigned    obj_cntr;

public:
    Aws::Utils::Logging::LogSystemInterface* LogSystem;

    s3_application();
    ~s3_application();
    
    static void init();
    static void shutdown();

    /**
     * List objects (keys) within an Amazon S3 bucket.
     */
    void backets_list();
    bool backets_check_create(const Aws::String& bucket_name);
    bool backets_check_create(const std::string& bucket_name);
    /**
     * Put an object into an Amazon S3 bucket
     */
    bool put_s3_object(const Aws::String& s3_bucket_name, 
        const Aws::String& s3_object_name, 
        const std::string& file_name);
};
