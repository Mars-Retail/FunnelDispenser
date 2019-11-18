# RootCertificates
This is a very simple library that can be used to store SSL certificates (in Base-64 encoded X.509 format) stored as strings and associate them with a domain. 

In this way you can use the right certificate to validate connections with different domains.

This may be needed, for example, if your device needs to fetch data from different sources and you don't want to manage the certificates directly inside your sketch.

SSL uses certificates to ensure that your connection is encrypted and that you are actually connecting to the server you specified in your connection URL. Certificates are "chained", one certificate may reference a higher-level certificate to grant his authenticity. The client must download and verify all the certificates in the chain until it reaches a well-know root certificate. 

Browsers and operating systems store those certificates in the filesystem or in a database, but this is not doable on Arduino. 

If you connect to a single server you may store just a single root certificate as a global variable in your code. If you need to connect to multiple different servers, under different internet domains, you will need to provide the right root certificate for each specific request. 

This library is designed to simplify this process. It keeps a list of domains and their root certificate and can return the right certificate from a specific URL (that may include protocol, username and password and an additional path).

Since certificates requires a few KB each the library can be customized to include only the certificates your specific application needs.

## How to use the library
The library contains only one function: **getCertificate**.
You can use it to retrieve a certificate string (mime64 encoded X509 certificate) from an url.
For example:
```C++
String googlecert;

googlecert=getCertificate("https://www.google.com");
```
You can also use more complex URLs that include paths and username/passwords.
```C++
String googlecert;

googlecert=getCertificate("https://www.google.com/search?q=arduino+processing+language");
```

If you use the library with the [ESP32 HttpClient implementation](https://github.com/espressif/arduino-esp32) you can pass the certificate directly to the **begin** method:
```C++
HttpClient secureclient;
String url="www.google.com";

secureclient.begin(url,getCertificate(url.c_str()));
```

## Reduce code size
By default the library includes all supported certificates. Each certificate will require a few kilobytes of program memory and this can became an issue on memory-constrained devices.

You can reduce the size by adding only specific certificates. 
To do this you need to edit [src/RootCertificates.h](https://github.com/VMinute/RootCertificates/blob/master/src/RootCertificates.h) uncommenting the line that defines **USE_SPECIFIC_CERTIFICATES**:
```C++
// uncomment this to select wich certificates you need using 
// defines inside your sketche
#define USE_SPECIFIC_CERTIFICATES
```
This will not include certificates by default, you'll have to add additional defines to enable the specific certificates you need, like:
```C++
#define CERTIFICATE_GOOGLE_COM
```
Unfortunately I haven't found a way to specify those defines at sketch level in a way that applies to both the sketch code and the library code so changing the header is currenly the only way to do this. The header file may be overwritten if you update the library from the IDE.
## Internals
The library has a list of domains and certificates. It strips down the URL removing schema (protocol), login information and any additional path, retrieving the server name. It uses this to search inside the list, if it doesn't find a valid entry it removes the first part of the server name (so "www.google.com" becomes "google.com") and searches again.
If no valid match is found, then a NULL is returned.
## Add new certificates
To add a new certificate you need to edit [src/RootCertificates.cpp](https://github.com/VMinute/RootCertificates/blob/master/src/RootCertificates.cpp), adding a define for the new domain:
and an entry in the certificates array, inside an **#ifdef** block:
```C++
#ifdef CERTIFICATE_GOOGLEUSERCONTENT_COM
  {
    "googleusercontent.com",
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n"
    "A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n"
    "Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n"
    "MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n"
    "A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n"
    "hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n"
    "v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n"
    "eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n"
    "tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n"
    "C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n"
    "zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n"
    "mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n"
    "V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n"
    "bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n"
    "3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n"
    "J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n"
    "291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n"
    "ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n"
    "AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n"
    "TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n"
    "-----END CERTIFICATE-----\n"  
  },
#endif
```
If you want your certificate to be included by default when the library is built, add a line inside the **"#ifndef USE_SPECIFIC_CERTIFICATES"** block at the beginning of the cpp file.
```C++
// Includes all supported domains (may increase code size quite a lot!)
#ifndef USE_SPECIFIC_CERTIFICATES
#define CERTIFICATE_GOOGLE_COM
#define CERTIFICATE_GOOGLEUSERCONTENT_COM
#endif
```
Keep also in mind that some certificates may be revoked if they are compromised, unfortunately the library has just static information, so it can't update security certificates automatically, you'll have to fix the code, removing or replacing certificates as needed, and then re-flash your updated sketch.
## Save a top-level certificate
You can easily save a certificate for an HTTPS connection using your browser. Open a url on the server you want to use and check that it will show a secure connection icon (usually a small lock). On Chrome you will see the lock and the name of the domain associated with the certificate. If you click on it you will see a pop-up window with security information about the site. Clicking on the "valid" link under the "Certificate" entry you can inspect the certificate. Then you can move to the "Certification Path" tab and see the full chain. You can select the top-level certificate and view it. On the "details" tab you will have a button that will allow you to save it. Save it as Mime-64 encoded pem file and open it with a text editor. You'll just need to copy the text to your cpp file, adding the double quotes and end-of-line symbols ("\n") at the end, as in the above sample. Rebuild your sketch and you should be able to safely connect to your server.
