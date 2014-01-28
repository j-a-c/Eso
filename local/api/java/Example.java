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

    public byte[] symmetricEncrypt(String setName, String secretMessage, int version)
    {
        return eso.encrypt(setName, secretMessage.getBytes(), 1);
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

        // Encrypt the secret message.
        byte[] encryptedMsg = example.symmetricEncrypt(setName, secretMessage, 1);
        System.out.println("Encrypted message: " + Arrays.toString(encryptedMsg));
    }

}
