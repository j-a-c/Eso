import EsoLocal.*;

import java.util.Arrays;

/**
 * @author Joshua A. Campbell
 *
 * Examples for the Eso Java client API.
 */
class Example 
{
    // Reference to the Eso local client.
    EsoLocal eso;

    Example(){}

    /**
     * An example of how to request the Eso local service.
     */
    public void getService()
    {
        try
        {
            eso = EsoLocal.getService();
        }
        catch(Exception e)
        {
            System.out.println(e);
            throw new RuntimeException();
        }
    }

    public byte[] symmetricEncrypt(String setName, byte[] secretMessage, int version)
    {
        return eso.encrypt(setName, secretMessage, 1);
    }

    public byte[] symmetricDecrypt(String setName, byte[] secretMessage, int version)
    {
        return eso.decrypt(setName, secretMessage, 1);
    }


    /**
     * Driver for the examples.
     */
    public static void main(String[] args)
    {
        Example example = new Example();

        // Attempt to connect to the Eso local service.
        example.getService();
        
        // Sample set name and a secret message.
        String setName = "com.joshuac.test.sym";
        String secretMessage = "Hello World";

        // Print the original message.
        System.out.println("Original message:" + secretMessage);
        System.out.println("Original bytes: " + secretMessage.getBytes());

        // Encrypt the secret message.
        byte[] encryptedMsg = example.symmetricEncrypt(setName, secretMessage.getBytes(), 1);
        System.out.println("Encrypted bytes: " + Arrays.toString(encryptedMsg));
        // Decrypt the encrypted message.
        byte[] decryptedMsg = example.symmetricDecrypt(setName, encryptedMsg, 1);
        secretMessage = new String(decryptedMsg);
        System.out.println("Decrypted message:" + secretMessage);
        System.out.println("Decrypted bytes: " + Arrays.toString(decryptedMsg));


    }

}
