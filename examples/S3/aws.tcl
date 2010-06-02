#!/usr/local/bin/tclsh8.6

package require S3;


set access_key_id "AKIAJZULOD5G7CESZHCA";
set secret_access_key "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";
set default_bucket "noaaport";

S3::Configure -accesskeyid $access_key_id \
    -secretaccesskey $secret_access_key \
    -default-bucket $default_bucket;
puts "Configured";

#S3::PutBucket;
#puts "Created";

#S3::Push -prefix test -directory test;
#puts "Pushed";

set b [S3::ListAllMyBuckets];
puts "Buckets: $b";

set b [S3::GetBucket];
puts "Listing: $b";

#S3::Pull -prefix test -directory test-copy

#S3::Delete -resource [lindex $b 0]
#puts "Deleted"

set b [S3::GetBucket];
puts "Listing: $b";




