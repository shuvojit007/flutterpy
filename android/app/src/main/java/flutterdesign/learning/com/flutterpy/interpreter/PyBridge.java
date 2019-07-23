package flutterdesign.learning.com.flutterpy.interpreter;

public class PyBridge {


    // Step 1 - This interface defines the type of messages I want to communicate to my owner
    public interface PythonStdListener {

        public void onInputWait();

        public void onNewLine(String data);
    }

    private static volatile PyBridge pyBridge;

    private PythonStdListener listener;

    private boolean input;
    private String inputValue = "";
    private int inputCount = 0;

    public int getInputCount() {
        return inputCount;
    }

    public void setInputCount(int inputCount) {
        this.inputCount = inputCount;
    }

    //private constructor.
    private PyBridge() {
        //Prevent form the reflection api.
        if (pyBridge != null) {
            throw new RuntimeException("Use getInstance() method to get the single instance of this class.");
        }
    }

    public static PyBridge getInstance() {
        //Double check locking pattern
        if (pyBridge == null) { //Check for the first time

            synchronized (PyBridge.class) {   //Check for the second time.
                //if there is no instance available... create new one
                if (pyBridge == null) pyBridge = new PyBridge();
            }
        }

        return pyBridge;
    }

    // Assign the listener implementing events interface that will receive the events
    public void setPythonStdListener(PythonStdListener listener) {
        this.listener = listener;
    }

    public boolean isInput() {
        return input;
    }

    public void setInput(boolean input) {
        this.input = input;
    }

    public String getInputValue() {
        return inputValue;
    }

    public void setInputValue(String inputValue) {
        this.inputValue = inputValue;
    }


    /**
     * Initializes the Python interpreter.
     *
     * @param datapath the location of the extracted python files
     * @return error code
     */
    public native int start(String datapath);

    /**
     * Stops the Python interpreter.
     *
     * @return error code
     */
    public native int stop();

    /**
     * Sends a string payload to the Python interpreter.
     *
     * @param payload the payload string
     * @return a string with the result
     */
    public native int call(String payload);

    public void messageMe(String text) {
        if (listener != null) {
            listener.onNewLine(text);
        } else {
            System.out.println("work out: " + text);
        }

    }

    public String inputGet() {
        System.out.println("input get");

        if (listener != null) {
            if (!isInput()) {
                listener.onInputWait();
                setInput(true);
                setInputValue("");
                System.out.println("Trigered");
            }
        }

        if (getInputValue().contains(":ok")) {
            setInput(false);
        }

        return getInputValue();
    }

    // Load library
    static {
        System.loadLibrary("pybridge");
    }

}
