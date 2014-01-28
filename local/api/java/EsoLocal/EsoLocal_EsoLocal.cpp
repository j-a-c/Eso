#include <jni.h>
#include <iostream>
#include <string>

#include "EsoLocal_EsoLocal.h"

#include "../../../config/esol_config.h"
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
 * Contacts the local daemon and requests in_msg to be encrypted using the
 * credentials from set in_set.
 */
JNIEXPORT jbyteArray JNICALL Java_EsoLocal_EsoLocal_encrypt(JNIEnv *env, 
        jobject obj, jstring in_set, jbyteArray in_msg, jint version)
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
        int len = env->GetArrayLength(in_msg);
        unsigned char* buf = new unsigned char[len];
        env->GetByteArrayRegion(in_msg, 0, len, reinterpret_cast<jbyte*>(buf));

        // Send encryption parameters.
        std::string msg{set_name};
        msg += MSG_DELIMITER;
        msg += std::to_string((int) version);
        msg += MSG_DELIMITER;
        msg += std::string{(char*)buf};
        uds_stream.send(msg);

        // TODO Receive the encrypted string.
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
    catch(std::exception e)
    {
        // TODO
    }

}
