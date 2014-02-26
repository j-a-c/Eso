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
     * enum itself. See crypto/constants.h for the constant's values.
     */
    public static enum Hash
    {
        DEFAULT, SHA1, SHA256;
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
     *
     * @return True if the service could be reached, false otherwise.
     */
    private native boolean pingEsoLocal();

    /**
     * Encrypts the data given using the specified version of the credentials
     * found at the given set.
     *
     * @param set The set containing the credentials.
     * @param data The data to encrypt.
     * @param version The version of the credentials to use.
     *
     * @return The encrypted data.
     */
    public native byte[] encrypt(String set, int version, byte[] data);

    /**
     * Decrypts the data given using the specified version of the credentials
     * found at the given set.
     *
     * @param set The set containing the credentials.
     * @param data The data to decrypt.
     * @param version The version of the credentials to use.
     *
     * @return The decrypted data.
     */
    public native byte[] decrypt(String set, int version, byte[] data);

    /**
     * Computes the message authentication code of the data using the specified
     * version of the credentials found at the given set and the specified hash
     * function. This function should only be called from the corresponding
     * wrapper function.
     *
     * @param set The set containing the credentials.
     * @param version The version of the credentials to use.
     * @param data The data to compute the HMAC for.
     * @param hash The hash function to use.
     *
     * @return The HMAC.
     *
     */
    private native byte[] hmac(String set, int version, byte[] data, int hash);

    /**
     * Wrapper around the native method because it is easier to pass the
     * ordinal of the enum than the enum itself.
     *
     * @param set The set containing the credentials.
     * @param version The version of the credentials to use.
     * @param data The data to compute the HMAC for.
     * @param hash The hash function to use.
     *
     * @return The HMAC.
     */
    public byte[] hmac(String set, int version, byte[] data, Hash hash)
    {
        return hmac(set, version, data, hash.ordinal());
    }

    /**
     * Computes the signaure of the data using the specified algorithm.
     *
     * @param set The set containing the credentials.
     * @param version The version of the credentials to use.
     * @param data The data to sign.
     * @param algo The hash function to use.
     *
     * @return The signature.
     */
    private native byte[] sign(String set, int version, byte[] data, int algo); 

    /**
     * Wrapper around the native method because it is easier to pass the
     * ordinal of the enum than the enum itself.
     *
     * @param set The set containing the credentials.
     * @param version The version of the credentials to use.
     * @param data The data to sign.
     * @param algo The hash function to use.
     *
     * @return The signature.
     */
    public byte[] sign(String set, int version, byte[] data, Hash algo)
    {
        return sign(set, version, data, algo.ordinal());
    }


    /**
     * Verifies the signature.
     * 
     * @param set The set containing the credentials.
     * @param version The version of the credentials to use.
     * @param sig The signature to verify.
     * @param data The data to compare against.
     * @param algo The hash function to use.
     *
     * @return True if the signature was verified, false otherwise.
     */
    private native boolean verify(String set, int version, byte[] sig, byte[] data, int algo);

    /**
     * Wrapper around the native method because it is easier to pass the
     * ordinal of the enum that the enum itself.
     *
     * @param set The set containing the credentials.
     * @param version The version of the credentials to use.
     * @param sig The signature to verify.
     * @param data The data to compare against.
     * @param algo The hash function to use.
     *
     * @return True if the signature was verified, false otherwise.
     */
    public boolean verify(String set, int version, byte[] sig, byte[] data, Hash algo)
    {
        return verify(set, version, sig, data, algo.ordinal()); 
    }


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
