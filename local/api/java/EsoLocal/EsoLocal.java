package EsoLocal;

import java.lang.reflect.Field;

/**
 * @author Joshua A. Campbell
 *
 * The local Eso service. This service is used when the client does not have
 * permission to retrieve the credentials.
 */
public class EsoLocal
{
    // A little hack to find the dynamic library.
    static 
    {
        // TODO need to fix how the to find the library.
        // Perhaps find the JAR file location and go from there?
        System.setProperty("java.library.path", "./lib" );

        try
        {
            Field fieldSysPath = 
                ClassLoader.class.getDeclaredField("sys_paths");

            fieldSysPath.setAccessible(true);
            fieldSysPath.set(null, null);
        }
        catch(Exception e){}

        System.loadLibrary("esol"); 
    }

    /**
     * Native method that returns true if the Eso local client can be reached.
     */
    private native boolean pingEsoLocal();

    /**
     * TODO
     */
    public native byte[] encrypt(String set, byte[] data);

    /**
     * TODO
     */
    public native byte[] decrypt(String set, byte[] data);

    /**
     * TODO
     */
    public native byte[] sign(String set, byte[] data);

    /**
     * TODO
     */
    public native byte[] hmac(String set, byte[] data);

    /**
     * TODO
     */
    public native byte[] verify(String set, byte[] data);

    /**
     * Private constructor to force user to test for service.
     * Will throw a RuntimeException if the local service cannot be reached.
     *
     * @throws EsoLocalConnectionException
     */
    private EsoLocal() throws EsoLocalConnectionException
    {
        if (!pingEsoLocal())
            throw new EsoLocalConnectionException("Cannot reach Eso service.");
    }

    /**
     * Attempts to contact the Eso local service.
     * If the service is reached, an EsoLocal instance is returned.
     * Throws a RuntimeException if the service cannot be reached.
     *
     * @throws EsoLocalConnectionException
     */
    public static EsoLocal getService() throws EsoLocalConnectionException
    {
        return new EsoLocal();
    }
}
