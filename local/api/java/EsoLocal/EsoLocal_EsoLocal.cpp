#include <jni.h>
#include <iostream>
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
        jobject obj, jstring in_set, jbyteArray in_data, jint version)
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

        // Receive the encrypted string.
        std::string encryption = uds_stream.recv();

        // Convert encryption from native to Java.
        len = encryption.length();
        jbyteArray encArray = env->NewByteArray(len);
        env->SetByteArrayRegion(encArray, 0, len, (jbyte*)encryption.c_str());

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
  (JNIEnv *env, jobject obj, jstring in_set, jbyteArray in_data, jint version)
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
        std::string msg{set_name};
        msg += MSG_DELIMITER;
        msg += std::to_string((int) version);
        msg += MSG_DELIMITER;
        msg += std::string{(char*)data};
        uds_stream.send(msg);

        // Receive the decrypted string.
        std::string decryption = uds_stream.recv();

        // Convert decryption from native to Java.
        len = decryption.length();
        jbyteArray decArray = env->NewByteArray(len);
        env->SetByteArrayRegion(decArray, 0, len, (jbyte*)decryption.c_str());

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
 * Contains the local daemon and requests an HMAC.
 */
JNIEXPORT jbyteArray JNICALL Java_EsoLocal_EsoLocal_hmac
  (JNIEnv *env, jobject jobj, jstring in_set, jbyteArray in_data, jint version,
   jint hash)
{
    UDS_Socket uds_socket{std::string{ESOL_SOCKET_PATH}};

    try
    {
        // Send decrypt request message.
        UDS_Stream uds_stream = uds_socket.connect();
        uds_stream.send(REQUEST_HMAC);

        // Get the set name from the input parameters.
        jboolean isCopy;
        const char *set_name = env->GetStringUTFChars(in_set, &isCopy);

        // Get the data array.
        int len = env->GetArrayLength(in_data);
        unsigned char* data = new unsigned char[len];
        env->GetByteArrayRegion(in_data, 0, len, reinterpret_cast<jbyte*>(data));

        // Send decryption parameters.
        std::string msg{set_name};
        msg += MSG_DELIMITER;
        msg += std::to_string((int) version);
        msg += MSG_DELIMITER;
        msg += std::string{(char*)data};
        msg += MSG_DELIMITER;
        msg += std::to_string((int) hash);

        uds_stream.send(msg);

        // Receive the decrypted string.
        std::string hmac = uds_stream.recv();

        // Convert decryption from native to Java.
        len = hmac.length();
        jbyteArray jhmac = env->NewByteArray(len);
        env->SetByteArrayRegion(jhmac, 0, len, (jbyte*)hmac.c_str());

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
