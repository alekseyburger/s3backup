#include <iostream>
#include <fstream>

#include "s3_application.hpp"

Aws::SDKOptions s3_application::options;
const char* s3_application::app_name = "mysrv";
unsigned    s3_application::obj_cntr = 0;

s3_application::s3_application()
{
    if(!s3_application::obj_cntr) {
        s3_application::init();
    }
    s3_application::obj_cntr++;

    LogSystem = Aws::Utils::Logging::GetLogSystem();

    p_credentials = new Aws::Auth::AWSCredentials(accessKeyId, secretKey);
    configuration.scheme = Aws::Http::Scheme::HTTP;

    p_s3_client = new Aws::S3::S3Client(*p_credentials, configuration);
    p_s3_client->OverrideEndpoint(endpoint);
}

s3_application::~s3_application()
{
    delete p_s3_client;
    delete p_credentials;

    s3_application::obj_cntr--;
    if(!s3_application::obj_cntr) {
        s3_application::shutdown();
    }
}

void s3_application::init()
{
    Aws::InitAPI(s3_application::options);

    Aws::Utils::Logging::InitializeAWSLogging(
    Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
            app_name, Aws::Utils::Logging::LogLevel::Trace, app_name));
}

void s3_application::shutdown()
{
    Aws::Utils::Logging::ShutdownAWSLogging();
    Aws::ShutdownAPI(options);    
}

/**
 * List objects (keys) within an Amazon S3 bucket.
 */
void s3_application::backets_list()
{
    auto outcome = p_s3_client->ListBuckets();

    if (outcome.IsSuccess())
    {
        LogSystem->Log(Aws::Utils::Logging::LogLevel::Info, app_name,
            "Your Amazon S3 buckets:");

        Aws::Vector<Aws::S3::Model::Bucket> bucket_list =
            outcome.GetResult().GetBuckets();

        for (auto const &bucket : bucket_list)
        {
            LogSystem->Log(Aws::Utils::Logging::LogLevel::Info, app_name,
                bucket.GetName().c_str());
        }
    }
    else
    {
        LogSystem->Log(Aws::Utils::Logging::LogLevel::Error, app_name,
            ("ListBuckets error: " + outcome.GetError().GetExceptionName() + " - "
            + outcome.GetError().GetMessage()).c_str());
    }
}

bool s3_application::backets_check_create(const Aws::String& bucket_name)
{
    // try to find backets
    auto outcome = p_s3_client->ListBuckets();

    if (!outcome.IsSuccess())
    {   
        Aws::String msg = "ListBuckets error: " + 
            outcome.GetError().GetExceptionName() + " - "
            + outcome.GetError().GetMessage();
        LogSystem->Log(Aws::Utils::Logging::LogLevel::Info, app_name, msg.c_str());        
        return false;
    }

    Aws::Vector<Aws::S3::Model::Bucket> bucket_list =
        outcome.GetResult().GetBuckets();

    for (auto const &bucket : bucket_list)
    {
        if (bucket.GetName().compare(bucket_name) == 0) {
            // backet is found
            return true;
        }
    }

    // Create the bucket
    // Set up create request
    Aws::S3::Model::CreateBucketRequest request;
    request.SetBucket(bucket_name);

    // Create the bucket
    auto create_res = p_s3_client->CreateBucket(request);
    if (!create_res.IsSuccess())
    {
        auto err = create_res.GetError();
        Aws::String msg = "ERROR: CreateBucket: " + 
            err.GetExceptionName() + ": " 
            + err.GetMessage();
        LogSystem->Log(Aws::Utils::Logging::LogLevel::Error, app_name, msg.c_str());
        return false;
    }

    return true;
}

bool s3_application::backets_check_create(const std::string& bucket_name)
{
    return backets_check_create(Aws::Utils::StringUtils::to_string(bucket_name));
}

/**
 * Put an object into an Amazon S3 bucket
 */
bool s3_application::put_s3_object(const Aws::String& s3_bucket_name, 
    const Aws::String& s3_object_name, 
    const std::string& file_name)
{
    // Set up request
    Aws::S3::Model::PutObjectRequest object_request;

    object_request.SetBucket(s3_bucket_name);
    object_request.SetKey(s3_object_name);
    const std::shared_ptr<Aws::IOStream> input_data = 
        Aws::MakeShared<Aws::FStream>("SampleAllocationTag", 
                                    file_name.c_str(), 
                                    std::ios_base::in | std::ios_base::binary);
    object_request.SetBody(input_data);

    // Put the object
    auto put_object_outcome = p_s3_client->PutObject(object_request);
    if (!put_object_outcome.IsSuccess()) {
        auto error = put_object_outcome.GetError();
        Aws::String msg = "ERROR: " + error.GetExceptionName() +  ": " + error.GetMessage();
        LogSystem->Log(Aws::Utils::Logging::LogLevel::Error,
            app_name, 
            msg.c_str());
        return false;
    }
    return true;
}
