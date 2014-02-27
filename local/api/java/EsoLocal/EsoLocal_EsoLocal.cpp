#include <jni.h>
#include <iostream> // Only for debug purposes.
#include <string>

#include "EsoLocal_EsoLocal.h"

#include "../../../config/esol_config.h"
#include "../../../../database/credential.h"
#include "../../../../global_config/global_config.h"
#include "../../../../global_config/message_config.h"
#include "../../../../socket/exception.h"
#include "../../../../socket/uds_socket.h"
#include "../../../../socket/uds_stream.h"


/**
 * Native method for Java class EsoLocal.EsoLocal.
 * Returns true if the Eso local client can be reached.
 */
JNIEXPORT jboolean JNICALL Java_EsoLocal_EsoLocal_pingEsoLocal(JNIEnv *env,
        jobject obj)

{
    UDS_Socket uds_socket{std::string{ESOL_SOCKET_PATH}};

    // Attempt to connect to the local client.
    try
    {
        // Ping the local client.
        UDS_Stream uds_stream = uds_socket.connect();
        uds_stream.send(PING);

        if (uds_stream.recv() == PING)
            return JNI_TRUE;
        else 
            return JNI_FALSE;
    }
    catch(std::exception e)
    {
        return JNI_FALSE;
    }
}


/*
 * Native method for EsoLocal.EsoLocal_EsoLocal.
 * Contacts the local daemon and requests in_data to be encrypted using the
 * credentials from set in_set.
 */
JNIEXPORT jbyteArray JNICALL Java_EsoLocal_EsoLocal_encrypt(JNIEnv *env, 
        jobject obj, jstring in_set, jint version, jbyteArray in_data)
{
    UDS_Socket uds_socket{std::string{ESOL_SOCKET_PATH}};

    try
    {
        // Send encrypt request message.
        UDS_Stream uds_stream = uds_socket.connect();
        uds_stream.send(REQUEST_ENCRYPT);

        // Get the set name from the input parameters.
        jboolean isCopy;
        const char *set_name = env->GetStringUTFChars(in_set, &isCopy);

        // Get the data array.
        int len = env->GetArrayLength(in_data);
        unsigned char* buf = new unsigned char[len];
        env->GetByteArrayRegion(in_data, 0, len, reinterpret_cast<jbyte*>(buf));

        // Send encryption parameters.
        std::string msg{set_name};
        msg += MSG_DELIMITER;
        msg += std::to_string((int) version);
        msg += MSG_DELIMITER;
        msg += std::string{(char*)buf};
        uds_stream.send(msg);

        // Receive the encrypted data.
        char_vec encryption = uds_stream.recv();

        // Convert encryption from native to Java.
        len = encryption.size();
        jbyteArray encArray = env->NewByteArray(len);
        env->SetByteArrayRegion(encArray, 0, len, (jbyte*)&encryption[0]);

        // Release the set_name.
        env->ReleaseStringUTFChars(in_set, set_name);
        // Free the data message. 
        free(buf);

        return encArray; 
    }
    catch (std::exception e)
    {
        // TODO
    }

}

/*
 * Native method for EsoLocal.EsoLocal_EsoLocal.
 * Contacts the local daemon and requests in_data to be decrypted using the
 * credentials from set in_set.
 */
JNIEXPORT jbyteArray JNICALL Java_EsoLocal_EsoLocal_decrypt
  (JNIEnv *env, jobject obj, jstring in_set, jint version, jbyteArray in_data)
{
    UDS_Socket uds_socket{std::string{ESOL_SOCKET_PATH}};

    try
    {
        // Send decrypt request message.
        UDS_Stream uds_stream = uds_socket.connect();
        uds_stream.send(REQUEST_DECRYPT);

        // Get the set name from the input parameters.
        jboolean isCopy;
        const char *set_name = env->GetStringUTFChars(in_set, &isCopy);

        // Get the data array.
        int len = env->GetArrayLength(in_data);
        unsigned char* data = new unsigned char[len];
        env->GetByteArrayRegion(in_data, 0, len, reinterpret_cast<jbyte*>(data));

        // Send decryption parameters.
        uds_stream.send(set_name);
        uds_stream.send(std::to_string(version));
        uds_stream.send(char_vec{&data[0], &data[0]+len});

        // Receive the decrypted data.
        char_vec decryption = uds_stream.recv();

        // Convert decryption from native to Java.
        len = decryption.size();
        jbyteArray decArray = env->NewByteArray(len);
        env->SetByteArrayRegion(decArray, 0, len, (jbyte*)&decryption[0]);

        // Release the set_name.
        env->ReleaseStringUTFChars(in_set, set_name);
        // Free the data message. 
        free(data);

        return decArray; 
    }
    catch (std::exception e)
    {
        // TODO
    }

}

/*
 * Native method for EsoLocal.EsoLocal_EsoLocal.
 * Contacts the local daemon and requests an HMAC.
 */
JNIEXPORT jbyteArray JNICALL Java_EsoLocal_EsoLocal_hmac
  (JNIEnv *env, jobject jobj, jstring in_set, jint version, jbyteArray in_data,
   jint hash)
{
    UDS_Socket uds_socket{std::string{ESOL_SOCKET_PATH}};

    try
    {
        // Send HMAC request message.
        UDS_Stream uds_stream = uds_socket.connect();
        uds_stream.send(REQUEST_HMAC);

        // Get the set name from the input parameters.
        jboolean isCopy;
        const char *set_name = env->GetStringUTFChars(in_set, &isCopy);

        // Get the data array.
        int len = env->GetArrayLength(in_data);
        unsigned char* data = new unsigned char[len];
        env->GetByteArrayRegion(in_data, 0, len, reinterpret_cast<jbyte*>(data));

        // Send HMAC parameters.
        uds_stream.send(set_name);
        uds_stream.send(std::to_string((int) version));
        uds_stream.send(char_vec{&data[0], &data[0]+len});
        uds_stream.send(std::to_string((int) hash));

        // Receive the HMAC'd data.
        char_vec hmac = uds_stream.recv();

        // Convert decryption from native to Java.
        len = hmac.size();
        jbyteArray jhmac = env->NewByteArray(len);
        env->SetByteArrayRegion(jhmac, 0, len, (jbyte*)&hmac[0]);

        // Release the set_name.
        env->ReleaseStringUTFChars(in_set, set_name);
        // Free the data message. 
        free(data);

        return jhmac; 

    }
    catch (std::exception e)
    {
        // TODO
    }

}

/*
 * Native method for EsoLocal.EsoLocal_EsoLocal.
 * Contacts the local daemon and requests a sign.
 */
JNIEXPORT jbyteArray JNICALL Java_EsoLocal_EsoLocal_sign
  (JNIEnv *env, jobject obj, jstring in_set, jint version, jbyteArray in_data,
   jint hash)
{
    UDS_Socket uds_socket{std::string{ESOL_SOCKET_PATH}};

    try
    {
        // Send sign request message.
        UDS_Stream uds_stream = uds_socket.connect();
        uds_stream.send(REQUEST_SIGN);

        // Get the set name from the input parameters.
        jboolean isCopy;
        const char *set_name = env->GetStringUTFChars(in_set, &isCopy);

        // Get the data array.
        int len = env->GetArrayLength(in_data);
        unsigned char* buf = new unsigned char[len];
        env->GetByteArrayRegion(in_data, 0, len, reinterpret_cast<jbyte*>(buf));

        // Send sign parameters.
        uds_stream.send(set_name);
        uds_stream.send(std::to_string((int)version));
        uds_stream.send(char_vec{&buf[0], &buf[0]+len});
        uds_stream.send(std::to_string((int)hash));

        // Receive the signed data.
        char_vec signature = uds_stream.recv();

        // Convert signature from native to Java.
        len = signature.size();
        jbyteArray sigArray = env->NewByteArray(len);
        env->SetByteArrayRegion(sigArray, 0, len, (jbyte*)&signature[0]);

        // Release the set_name.
        env->ReleaseStringUTFChars(in_set, set_name);
        // Free the data message. 
        free(buf);

        return sigArray; 
    }
    catch (std::exception e)
    {
        // TODO
    }
 
}

/*
 * Native method for EsoLocal.EsoLocal_EsoLocal.
 * Contacts the local daemon and requests a verify.
 */
JNIEXPORT jboolean JNICALL Java_EsoLocal_EsoLocal_verify
  (JNIEnv *env, jobject obj, jstring in_set, jint version, jbyteArray in_sig, 
   jbyteArray in_data, jint hash)
{
    UDS_Socket uds_socket{std::string{ESOL_SOCKET_PATH}};

    try
    {
        // Send verification request message.
        UDS_Stream uds_stream = uds_socket.connect();
        uds_stream.send(REQUEST_VERIFY);

        // Get the set name from the input parameters.
        jboolean isCopy;
        const char *set_name = env->GetStringUTFChars(in_set, &isCopy);

        // Get the signature array.
        int siglen = env->GetArrayLength(in_sig);
        unsigned char* sigbuf = new unsigned char[siglen];
        env->GetByteArrayRegion(in_sig, 0, siglen, reinterpret_cast<jbyte*>(sigbuf));

        // Get the data array.
        int datalen = env->GetArrayLength(in_data);
        unsigned char* databuf = new unsigned char[datalen];
        env->GetByteArrayRegion(in_data, 0, datalen, reinterpret_cast<jbyte*>(databuf));

        // Send verification parameters.
        uds_stream.send(set_name);
        uds_stream.send(std::to_string((int)version));
        uds_stream.send(char_vec{&sigbuf[0], &sigbuf[0]+siglen});
        uds_stream.send(char_vec{&databuf[0], &databuf[0]+datalen});
        uds_stream.send(std::to_string((int)hash));

        // Receive the validity.
        char_vec valid_msg = uds_stream.recv();

        // Release the set_name.
        env->ReleaseStringUTFChars(in_set, set_name);
        // Free the allocated memory. 
        free(sigbuf);
        free(databuf);

        // If the value is logically true, return true.
        if (valid_msg[0])
            return JNI_TRUE; 
        else
            return JNI_FALSE;
    }
    catch (std::exception e)
    {
        // TODO
    }
}

