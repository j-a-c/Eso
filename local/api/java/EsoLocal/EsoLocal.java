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
    /**
     * Allowable hashes. The ordinal values of the enums correspond to their
     * values in the native library. It is easier to pass the ordinal than the
     * enum itself.
     */
    public static enum Hash
    {
        DEFAULT, SHA1;
    }

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
     * Encrypts the data given using the specified version of the credentials
     * found at the given set.
     *
     * @param set The set containing the credentials.
     * @param data The data to encrypt.
     * @param version The version of the credentials to use.
     */
    public native byte[] encrypt(String set, byte[] data, int version);

    /**
     * Decrypts the data given using the specified version of the credentials
     * found at the given set.
     *
     * @param set The set containing the credentials.
     * @param data The data to decrypt.
     * @param version The version of the credentials to use.
     */

    public native byte[] decrypt(String set, byte[] data, int version);

    /**
     * TODO
     */
    public native byte[] sign(String set, byte[] data, int version); 

    /**
     * Computes the message authentication code of the data using the specified
     * version of the credentials found at the given set and the specified hash
     * function. This function should only be called from the corresponding
     * wrapper function.
     *
     * @param set The set containing the credentials.
     * @param data The data to compute the HMAC for.
     * @param version The version of the credentials to use.
     * @param hash The hash function to use.
     *
     */
    private native byte[] hmac(String set, byte[] data, int version, int hash);

    /**
     * Wrapper around the native method because it is easier to pass the
     * ordinal of the enum than the enum itself.
     */
    public byte[] hmac(String set, byte[] data, int version, Hash hash)
    {
        return hmac(set, data, version, hash.ordinal());
    }

    /**
     * TODO
     */
    public native byte[] verify(String set, byte[] data, int version);

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
