/*******************************************************************************
 * Copyright (c) 2017 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Lokesh Haralakatta  -  Initial implementation
 *    Lokesh Haralakatta  -  Added Logging Feature
 *******************************************************************************/

#include "iotf_network_tls_wrapper.h"

//Character strings to hold log header and log message to be dumped.
char logHdr[LOG_BUF];
char logStr[LOG_BUF];

/** Function to initialize Network structure with defualt values and network functions
* @param - Address of Network Structure Variable
* @return - void
**/
void NewNetwork(Network* n)
{
       sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
       LOG(logHdr,"entry::");

       n->my_socket = 0;
       n->mqttread = network_read;
       n->mqttwrite = network_write;
       n->disconnect = network_disconnect;
       n->TLSConnectData.pServerCertLocation = NULL;
       n->TLSConnectData.pRootCACertLocation = NULL;
       n->TLSConnectData.pDeviceCertLocation = NULL;
       n->TLSConnectData.pDevicePrivateKeyLocation = NULL;
       n->TLSConnectData.pDestinationURL = NULL;

       sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
       LOG(logHdr,"exit::");
}

/** Function to check whether the given timer is expired or not
* @param - Address of Timer
* @return - 0 or 1
**/
 char expired(Timer* timer)
 {
 	struct timeval now, res;
 	gettimeofday(&now, NULL);
 	timersub(&timer->end_time, &now, &res);
 	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
 }

 /** Function to update timer with given timeout value in milliseconds
 * @param - Address of Timer and timeout in milliseconds
 * @return - void
 **/
 void countdown_ms(Timer* timer, unsigned int timeout)
 {
 	struct timeval now;
 	gettimeofday(&now, NULL);
 	struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
 	timeradd(&now, &interval, &timer->end_time);
 }

 /** Function to update timer with given timeout value in seconds
 * @param - Address of Timer and timeout in seconds
 * @return - void
 **/
 void countdown(Timer* timer, unsigned int timeout)
 {
 	struct timeval now;
 	gettimeofday(&now, NULL);
 	struct timeval interval = {timeout, 0};
 	timeradd(&now, &interval, &timer->end_time);
 }

 /** Function to find and return left out time in Timer
 * @param - Address of Timer
 * @return - Left out time in milliseconds
 **/
 int left_ms(Timer* timer)
 {
 	struct timeval now, res;
 	gettimeofday(&now, NULL);
 	timersub(&timer->end_time, &now, &res);
 	////printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
 	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
 }

 /** Function to initialize the Timer Structure variable
 * @param - Address of Timer
 * @return - void
 **/
 void InitTimer(Timer* timer)
 {
 	timer->end_time = (struct timeval){0, 0};
 }

 /** Function to establish connection to given host address and port without SSL/TLS
 * @param - Address of Network Structure
 *        - Host Address to connect
 *        - Port number to connect
 * @return - 0 for SUCCESS
 *         - -1 for FAILURE
 **/
 int ConnectNetwork(Network* n, char* addr, int port)
 {
        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

 	int type = SOCK_STREAM;
 	struct sockaddr_in address;
 	int rc = -1;
 	sa_family_t family = AF_INET;
 	struct addrinfo *result = NULL;
 	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

 	if ((rc = getaddrinfo(addr, NULL, &hints, &result)) == 0)
 	{
 		struct addrinfo* res = result;

 		/* prefer ip4 addresses */
 		while (res)
 		{
 			if (res->ai_family == AF_INET)
 			{
 				result = res;
 				break;
 			}
 			res = res->ai_next;
 		}

 		if (res->ai_family == AF_INET)
 		{
 			address.sin_port = htons(port);
 			address.sin_family = family = AF_INET;
 			address.sin_addr = ((struct sockaddr_in*)(res->ai_addr))->sin_addr;

                        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
                        sprintf(logStr,"%s","ADDR FAMILY: AF_INET");
                        LOG(logHdr,logStr);
 		}
 		else
 			rc = -1;

 		freeaddrinfo(result);
 	}

 	if (rc == 0)
 	{
 		n->my_socket = socket(family, type, 0);

                sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
                sprintf(logStr,"Socket FD: %d",n->my_socket);
                LOG(logHdr,logStr);

 		if (n->my_socket != -1)
 		{
 			rc = connect(n->my_socket, (struct sockaddr*)&address, sizeof(address));

                        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
                        sprintf(logStr,"RC from connect - %d :",rc);
                        LOG(logHdr,logStr);
 		}
 	}

        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        sprintf(logStr,"rc = %d ",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

 	return rc;
 }

 /** Function to read data from the socket opened into provided buffer
 * @param - Address of Network Structure
 *        - Buffer to store the data read from socket
 *        - Expected number of bytes to read from socket
 *        - Timeout in milliseconds
 * @return - Number of Bytes read on SUCCESS
 *         - -1 on FAILURE
 **/
 int network_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
 {
        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

 	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
 	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
 	{
 		interval.tv_sec = 0;
 		interval.tv_usec = 100;
 	}

 	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

 	int bytes = 0;
 	while (bytes < len)
 	{
 		int rc = recv(n->my_socket, &buffer[bytes], (size_t)(len - bytes), 0);
 		if (rc == -1)
 		{
 			if (errno != ENOTCONN && errno != ECONNRESET)
 			{
                                sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
                                sprintf(logStr,"network_read failed while calling recv with message - %s\n",strerror(rc));
                                LOG(logHdr,logStr);
 				bytes = -1;
 				break;
 			}
 		}
 		else
 			bytes += rc;
 	}

        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        sprintf(logStr,"bytes - %d ",bytes);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

 	return bytes;
 }

 /** Function to write data to the socket opened present in provided buffer
 * @param - Address of Network Structure
 *        - Buffer storing the data to write to socket
 *        - Number of bytes of data to write to socket
 *        - Timeout in milliseconds
 * @return - Number of Bytes written on SUCCESS
 *         - -1 on FAILURE
 **/
 int network_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
 {
        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

 	struct timeval tv;
        int rc = -1;

 	tv.tv_sec = 0;  /* 30 Secs Timeout */
 	tv.tv_usec = timeout_ms * 1000;  // Not init'ing this can cause strange errors

 	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
 	rc = write(n->my_socket, buffer, len);
        if(rc < 0){
                sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
                sprintf(logStr,"network_write failed while calling write with message - %s\n",strerror(rc));
                LOG(logHdr,logStr);
                rc = -1;
        }

        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        sprintf(logStr,"rc = %d ",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

 	return rc;
 }

 /** Function used to close the opened socket for communication. If the given mode is quick start,
 it just closes the socket opened otherwise it calls teardown_tls function to cleanup mbedtls structures.
 * @param - Address of Network Structure
 *        - Connected Mode - Quickstart or Not
 * @return - void
 **/
 void network_disconnect(Network* n, int qsMode)
 {
        if(qsMode)
 	  close(n->my_socket);
        else
          teardown_tls(&n->TLSInitData,&n->TLSConnectData);
 }

 /** Function to initialize mbedtls structures. If useClientCerts flag is 1,
 * then certificate related structure gets initialized.
 * @param - Address of tls_init Structure
 *        - Flag to indicate whether to use client certificates or not
 * @return - 0 on SUCCESS
 *         - -1 on FAILURE
 **/
 int initialize_tls(tls_init_params *tlsInitParams, int useClientCerts){
        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        int rc=-1;
        mbedtls_net_init( &(tlsInitParams->server_fd) );
        mbedtls_ssl_init( &(tlsInitParams->ssl) );
        mbedtls_ssl_config_init( &(tlsInitParams->conf) );
        mbedtls_x509_crt_init( &(tlsInitParams->cacert) );
        mbedtls_ctr_drbg_init( &(tlsInitParams->ctr_drbg) );
        mbedtls_entropy_init( &(tlsInitParams->entropy) );
        if(useClientCerts){
            mbedtls_x509_crt_init(&(tlsInitParams->clicert));
            mbedtls_pk_init(&(tlsInitParams->pkey));
        }
        strcpy(tlsInitParams->clientName,"mbed_tls_client");

        if((rc = mbedtls_ctr_drbg_seed( &(tlsInitParams->ctr_drbg), mbedtls_entropy_func, &(tlsInitParams->entropy),
                            (const unsigned char *) tlsInitParams->clientName,
                            strlen( tlsInitParams->clientName) ) )!= 0){
                sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
                sprintf(logStr,"mbedtls_ctr_drbg_seed failed with return code = 0x%x",rc);
                LOG(logHdr,logStr);
        }

        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        sprintf(logStr,"rc = %d ",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

        return rc;
 }

 /** Function to display SSL related debug messages from mbedtls library
 * @param - Address of Context
 *        - Debug level in the range 0 to 4
 *        - File Handle
 *        - Line number in the code
 *        - Message to display
 * @return - Number of Bytes written on SUCCESS
 *         - -1 on FAILURE
 **/
 void tls_debug( void *ctx, int level,
                      const char *file, int line, const char *str )
 {
    ((void) level);
    fprintf( (FILE *) ctx, "%s:%04d: %s", file, line, str );
    fflush(  (FILE *) ctx  );
 }

 /** Function to connect to given server using SSL/TLS secured connection. If useClientCerts Flag
 * is set, then it uses the specified Client Side Certificates for communication.
 * @param - Address of tls_init_larams structure
 *        - Address of tls_connect_params structure
 *        - Server Address
 *        - Port Number
 *        - whether to use client side certificates or not
 * @return - 1 on SUCCESS
 *         - -1 on FAILURE
 **/
 int tls_connect(tls_init_params *tlsInitData,tls_connect_params *tlsConnectData,
                const char *server, const int port, int useClientCerts){

        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        int rc=-1;
        char str_port[10];
        sprintf(str_port,"%d",port);

        //Remove the comment from mbedtls_debug_set_threshold for debugging purpose
        //mbedtls_debug_set_threshold(4);

        if((rc = initialize_tls(tlsInitData,useClientCerts))!=0)
        {
            sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
            sprintf(logStr,"initialize_tls failed with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
        }
        if((rc = mbedtls_net_connect(&(tlsInitData->server_fd), server, str_port, MBEDTLS_NET_PROTO_TCP )) != 0){
            sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
            sprintf(logStr,"mbedtls_net_connect failed with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
        }
        if((rc = mbedtls_net_set_block(&(tlsInitData->server_fd)))!=0){
            sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
            sprintf(logStr,"mbedtls_net_set_block failed with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
        }

        if((rc = mbedtls_x509_crt_parse_file(&(tlsInitData->cacert),tlsConnectData->pServerCertLocation))!=0)
        {
            sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
            sprintf(logStr,"mbedtls_x509_crt_parse_file failed for Server CA certificate with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
        }

        if(useClientCerts){
          if((rc = mbedtls_x509_crt_parse_file(&(tlsInitData->cacert),tlsConnectData->pRootCACertLocation))!=0)
          {
            sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
            sprintf(logStr,"mbedtls_x509_crt_parse_file failed for Root CA certificate with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
          }
          if((rc = mbedtls_x509_crt_parse_file(&(tlsInitData->clicert), tlsConnectData->pDeviceCertLocation))!=0)
          {
            sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
            sprintf(logStr,"mbedtls_x509_crt_parse_file failed for Device Certificate with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
          }
          if((rc = mbedtls_pk_parse_keyfile(&(tlsInitData->pkey), tlsConnectData->pDevicePrivateKeyLocation, ""))!=0)
          {
            sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
            sprintf(logStr,"mbedtls_pk_parse_keyfile failed for Device Private Key with return code = 0x%x",-rc);
            LOG(logHdr,logStr);
            goto exit;
          }
          if((rc = mbedtls_ssl_conf_own_cert(&(tlsInitData->conf), &(tlsInitData->clicert),
                       &(tlsInitData->pkey)))!=0)
          {
              sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
              sprintf(logStr,"mbedtls_ssl_conf_own_cert failed with return code = 0x%x",-rc);
              LOG(logHdr,logStr);
              goto exit;
          }
          if((rc = mbedtls_ssl_set_hostname( &(tlsInitData->ssl), tlsConnectData->pDestinationURL)) != 0 )
          {
              sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
              sprintf(logStr,"mbedtls_ssl_set_hostname failed with rc = 0x%x",-rc);
              LOG(logHdr,logStr);
              goto exit;
          }
        }
        if((rc = mbedtls_ssl_config_defaults(&(tlsInitData->conf),MBEDTLS_SSL_IS_CLIENT,
                      MBEDTLS_SSL_TRANSPORT_STREAM,MBEDTLS_SSL_PRESET_DEFAULT ))!=0)
        {
                sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
                sprintf(logStr,"mbedtls_ssl_config_defaults failed with return code = 0x%x",-rc);
                LOG(logHdr,logStr);
                goto exit;
        }
        mbedtls_ssl_conf_max_version(&(tlsInitData->conf),MBEDTLS_SSL_MAJOR_VERSION_3,MBEDTLS_SSL_MINOR_VERSION_3);
        mbedtls_ssl_conf_min_version(&(tlsInitData->conf),MBEDTLS_SSL_MAJOR_VERSION_3,MBEDTLS_SSL_MINOR_VERSION_3);
        mbedtls_ssl_conf_authmode(&(tlsInitData->conf), MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_ca_chain(&(tlsInitData->conf), &(tlsInitData->cacert), NULL);
        mbedtls_ssl_conf_rng(&(tlsInitData->conf), mbedtls_ctr_drbg_random, &(tlsInitData->ctr_drbg));
        mbedtls_ssl_conf_dbg( &(tlsInitData->conf), tls_debug, stdout );
        mbedtls_ssl_set_bio(&(tlsInitData->ssl), &(tlsInitData->server_fd), mbedtls_net_send, mbedtls_net_recv, NULL );
        if((rc = mbedtls_ssl_setup(&(tlsInitData->ssl), &(tlsInitData->conf))) != 0 )
        {
                sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
                sprintf(logStr,"mbedtls_ssl_setup failed with rc = 0x%x",-rc);
                LOG(logHdr,logStr);
                goto exit;
        }
        while((rc = mbedtls_ssl_handshake(&(tlsInitData->ssl))) != 0 )
        {
           if( rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE )
           {
                sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
                sprintf(logStr,"mbedtls_ssl_handshake failed with rc = 0x%x",-rc);
                LOG(logHdr,logStr);
                if(rc == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED){
                  sprintf(logStr,"ssl_handshake failed with MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED");
                  LOG(logHdr,logStr);
                }
                sprintf(logStr,"ssl state = %d",tlsInitData->ssl.state);
                LOG(logHdr,logStr);
                sprintf(logStr,"ssl version = %s",mbedtls_ssl_get_version(&(tlsInitData->ssl)));
                LOG(logHdr,logStr);
                break;
            }
        }
  exit:
        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        sprintf(logStr,"rc = %d ",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

        return rc;
 }

 /** Function to read data from the secured tls socket
 * @param - Address of Network Structure
 *        - Buffer to store the read data from the secured socket
 *        - Number of bytes of data to read from secured socket
 *        - Timeout in milliseconds
 * @return - Number of Bytes read on SUCCESS
 *         - -1 on FAILURE
 **/
 int tls_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
 {
        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        int rc = -1;
        tls_init_params *tlsInitData = &(n->TLSInitData);
 	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
 	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
 	{
 		interval.tv_sec = 0;
 		interval.tv_usec = 100;
 	}
        setsockopt(tlsInitData->server_fd.fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));
 	int bytes = 0;
 	while (bytes < len)
 	{
 	        rc = mbedtls_ssl_read(&(tlsInitData->ssl), buffer, len);
                if(rc == 0 || rc == -76)
 		     break;
                else if(rc < 0 ){
                   sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
                   sprintf(logStr,"mbedtls_ssl_read failed with rc = %d",rc);
                   LOG(logHdr,logStr);
                   bytes = -1;
                   break;
                }

 		else
 			bytes += rc;
 	}

        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        sprintf(logStr,"bytes - %d ",bytes);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

 	return bytes;
 }

 /** Function to write data to the secured tls socket
 * @param - Address of Network Structure
 *        - Buffer storing the data to write to the secured socket
 *        - Number of bytes of data to write to secured socket
 *        - Timeout in milliseconds
 * @return - Number of Bytes written on SUCCESS
 *         - -1 on FAILURE
 **/
 int tls_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
 {
        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

 	struct timeval tv;
 	int rc = -1;
        tls_init_params *tlsInitData = &(n->TLSInitData);
 	tv.tv_sec = 0;  /* 30 Secs Timeout */
 	tv.tv_usec = timeout_ms * 1000;  // Not init'ing this can cause strange errors
 	setsockopt(tlsInitData->server_fd.fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
        if((rc = mbedtls_ssl_write(&(tlsInitData->ssl),buffer,len)) < 0){
                sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
                sprintf(logStr,"mbedtls_ssl_write failed with rc = %d",rc);
                LOG(logHdr,logStr);
        }

        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        sprintf(logStr,"rc = %d ",rc);
        LOG(logHdr,logStr);
        LOG(logHdr,"exit::");

 	return rc;
 }

 /** Function to clear off the mbedtls structures.
 * @param - Address of tls_init Structure
 *        - Address of tls_connect Structure
 * @return - void
 **/
void teardown_tls(tls_init_params *tlsInitParams,tls_connect_params* tlsConnectData){
        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        mbedtls_net_free( &(tlsInitParams->server_fd) );
        mbedtls_ssl_free( &(tlsInitParams->ssl) );
        mbedtls_ssl_config_free( &(tlsInitParams->conf) );
        mbedtls_ctr_drbg_free( &(tlsInitParams->ctr_drbg) );
        mbedtls_entropy_free( &(tlsInitParams->entropy) );
        mbedtls_x509_crt_free( &(tlsInitParams->cacert) );

        freeTLSConnectData(tlsConnectData);

        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"exit::");
 }

 /** Function to clear off the memory allocated for certificates location.
 * @param - Address of tls_connect Structure
 * @return - void
 **/
void freeTLSConnectData(tls_connect_params* tlsConnectData){
        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"entry::");

        freePtr(tlsConnectData->pServerCertLocation);
        freePtr(tlsConnectData->pRootCACertLocation);
        freePtr(tlsConnectData->pDeviceCertLocation);
        freePtr(tlsConnectData->pDevicePrivateKeyLocation);
        freePtr(tlsConnectData->pDestinationURL);

        sprintf(logHdr,"%s:%d:%s:",__FILE__,__LINE__,__func__);
        LOG(logHdr,"exit::");
}
