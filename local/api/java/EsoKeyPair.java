import java.security.KeyPair;
import java.security.interfaces.RSAPrivateCrtKey;
import java.security.interfaces.RSAPublicKey;


public class EsoKeyPair
{
    // Hack to set the library path so we can load the implentation library.
    static 
    {
        System.setProperty("java.library.path", ".");
         
        try
        {
            Field fieldSysPath = ClassLoader.class.getDeclaredField("sys_paths");
            fieldSysPath.setAccessible(true);
            fieldSysPath.set(null, null);
        }
        catch(Exception e){}

        // esoJava.dll (Windows) or libesoJava.so (Unixes)
        System.loadLibrary("esoJava"); 
    }


    private class EsoRSAPublicKey implements RSAPublicKey
    {
        byte[] publicExponent;
    
        BigInteger getPublicExponent()
        {
            return new BigInteger(publicExponent);
        }
    }

    // TODO
    private class EsoRSAPublicKey implements RSAPrivateCrtKey
    {

        //Returns the crtCoefficient.
        BigInteger	getCrtCoefficient()

        //Returns the primeExponentP.
        BigInteger	getPrimeExponentP()

        //BigInteger	getPrimeExponentQ()
        Returns the primeExponentQ.

        //BigInteger	getPrimeP()
        Returns the primeP.

        //BigInteger	getPrimeQ()
        Returns the primeQ.

        //Returns the public exponent.
        BigInteger	getPublicExponent()
    }

    private String setName;
    private int version;

    /**
     * Fills out the public and private exponents.
     * @return length of the exponents.
     */
    private native int getAsymmetricExponents(byte[] publicKey, byte[] privateKey);

    /**
     * Creates an EsoKeyPair that will be set to return the most recent version
     * of the KeyPair for the specified set.
     */
    public EsoKeyPair(String setName)
    {
        this.setName = setName;
        this.version = 0;
    }
    
    /**
     * Creates an EsoKeyPair that will return the specified version of the
     * KeyPair for the specified set.
     */
    public EsoKeyPair(String setName, int version)
    {
        this.setName = setName;
        this.version = version;
    }

    /**
     * Query the esod deamon for the key pair.
     * We only support RSA, so that is the default return KeyPair.
     * Requires the 'retrieve' permission.
     */
    public KeyPair getKeyPair()
    {
        getAsymmetricKeys(publicKey, privateKey);

        return new KeyPair(publicKey, privateKey);
    }
}
