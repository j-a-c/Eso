import EsoLocal.*;

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

    /**
     * Driver for the examples.
     */
    public static void main(String[] args)
    {
        Example example = new Example();

        // Attempt to connect to the Eso local service.
        example.getService();
    }

}
