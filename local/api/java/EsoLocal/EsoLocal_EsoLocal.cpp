#include <jni.h>
#include "EsoLocal_EsoLocal.h"
#include <iostream>

#include "../../../config/esol_config.h"
#include "../../../../global_config/message_config.h"
#include "../../../../socket/exception.h"
#include "../../../../socket/uds_socket.h"
#include "../../../../socket/uds_stream.h"

JNIEXPORT jboolean JNICALL Java_EsoLocal_EsoLocal_pingEsoLocal (JNIEnv *, jobject)

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
