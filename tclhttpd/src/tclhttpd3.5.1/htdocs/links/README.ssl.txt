While TclHttpd can support SSL, you will need to add a number of
software components to complete your SSL server.

At the base is either RSAREF or OpenSSL.  Within the United States there are
patent restrictions that limit you to using RSAREF from RSA Inc.  Actually,
you can also build OpenSSL with a "no patents" option.  Both of these packages
create a crypto library with the same interface.
	http://www.rsa.com
	http://www.openssl.org

Next comes the "TLS" Tcl extension, which uses the crypto library.
The development home page for TLS is
        http://sourceforge.net/projects/tls/
I have used the 1.4.1 version for a number of years, although
there is a recent 1.5.0 version.  At SourceForge there are
binary releases for Linux and Solaris that save you the chore
of building OpenSSL.

If you can run tclsh and
        package require tls
then you are almost ready to go.

Finally you need keys and certificates for your server.  OpenSSL comes with
a command-line utility called "openssl" that you can use to generate keys
and certificates.  The RSFREF utility is "sslc", but provides essentially
the same features.  The general process is that you generate a public-private
key pair (using the "genrsa" command for sslc or openssl)
    sslc genrsa -out skey.pem
Next you create a certificate request
    sslc req -config /path/to/ssl.cnf -new -nodex -out ./server.pem -key ./skey.pem
and send this to a certificate authority for signing. 
One example Certificate Authority is
    http://www.verisign.com

Once you get the signed certificate back, edit the tclhttpd.rc file so they
accurately record the location of your keyfile and certificate.
The server should then be able to listen for SSL connections on the https port.

You can also bootstrap yourself into your own CA by following the steps
outlined below.  This lets you sign your own certificate requests to
make valid certificates, but browsers will prompt users to validate the
key when they visit your site.

You'll need the "openssl" command line utility that's
built when you build openssl.

0. Edit the sample "openssl.cnf" file that's distributed with
   openssl, replacing the dummy company names, etc.  Copy
   it to an empty directory and run openssl from there.

1. generate a private key for your test CA
   openssl genrsa -out key1.pem
2. build a certificate request
   openssl req -x509 -nodes -out ca.pem -key key1.pem -new

You've now got a CA certificate "ca.pem".
It's "self-signed".
Its private key is "key1.pem".

We'll now make a server cert and we'll use the CA cert
we just made to sign it.

3. generate a private key for your server
   opensl genrsa -out key2.pem
4. build a cert request
  openssl req -x509 -nodes -out s.pem -key key2.pem -new
5. build the server certificate
  openssl ca -keyfile key1.pem -cert ca.pem -in s.pem
6. rename the output, which is something like "01.pem"
  back to "s.pem" (it's OK to clobber the s.pem you made earlier)

You've now got a server certificate "s.pem".
Its private key is key2.pem.
It's signed by your own CA.

You can make any number of certs by repeating steps
three through six again with different file names.


