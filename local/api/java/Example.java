import EsoLocal.*;

import java.util.Arrays;
import java.lang.StringBuilder;

/**
 * @author Joshua A. Campbell
 *
 * Examples for the Eso Java client API.
 */
class Example 
{
    
    /**
     * Driver for the examples.
     */
    public static void main(String[] args)
    {
        Example example = new Example();

        // Reference to the Eso local client.
        EsoLocal eso;

        // Attempt to connect to the Eso local service.
        try
        {
            eso = EsoLocal.getService();
        }
        catch(Exception e)
        {
            System.out.println(e);
            throw new RuntimeException();
        }
        
        // Sample set name, a secret message, and version.
        String setName = "com.joshuac.test.sym";
        String message = "Hello World";
        int version = 1;

        System.out.println("=====");
        System.out.println("Symmetric encrypt/decrypt example:");
        System.out.println("-----");

        // Print the original message.
        System.out.println("Original message: " + message);
        System.out.println("Original bytes: " + Arrays.toString(message.getBytes()));

        System.out.println("-----");

        // Encrypt the secret message.
        byte[] encryptedMsg = eso.encrypt(setName, message.getBytes(), version);
        System.out.println("Encrypted bytes: " + Arrays.toString(encryptedMsg));

        System.out.println("-----");

        // Decrypt the encrypted message.
        byte[] decryptedMsg = eso.decrypt(setName, encryptedMsg, version);
        message = new String(decryptedMsg);
        System.out.println("Decrypted message: " + message);
        System.out.println("Decrypted bytes: " + Arrays.toString(decryptedMsg));

        System.out.println("====");
        System.out.println("HMAC example:");
        System.out.println("-----");

        // HMAC using SHA-1.
        byte[] hmacMsg = eso.hmac(setName, message.getBytes(), version, EsoLocal.Hash.SHA1);
        StringBuilder builder = new StringBuilder();
        for (byte b : hmacMsg)
            builder.append(String.format("%02x", b));
        System.out.println("HMAC: " + Arrays.toString(hmacMsg));
        System.out.println("Hex: " + builder.toString());

        System.out.println("====");


    }

}
