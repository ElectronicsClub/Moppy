package electronicsclub.moppyproject.testing;

import electronicsclub.moppyproject.outputs.MoppyPlayerOutput;
import gnu.io.CommPortIdentifier;
import gnu.io.NoSuchPortException;
import gnu.io.PortInUseException;
import gnu.io.SerialPort;
import gnu.io.SerialPortEvent;
import gnu.io.SerialPortEventListener;
import gnu.io.UnsupportedCommOperationException;

import java.io.IOException;
import java.io.InputStream;
import java.util.TooManyListenersException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * 
 * @author Graham Campbell
 * @author Michael Marley
 * @author SammyIAm
 * 
 */
public class MoppyCOMsumer implements Runnable, SerialPortEventListener {

    SerialPort currentPort = null;
    InputStream currentInputStream = null;
    boolean running = false;
    FloppySimWindow fsw;

    public MoppyCOMsumer(FloppySimWindow fsw) {
        this.fsw = fsw;
    }

    @Override
    public void run() {
        try {
            Thread.sleep(30000);
        } catch (InterruptedException ex) {
            Logger.getLogger(MoppyCOMsumer.class.getName()).log(Level.SEVERE,
                    null, ex);
        }
    }

    public void openPort(String portId) throws NoSuchPortException,
            PortInUseException, IOException, TooManyListenersException,
            UnsupportedCommOperationException {
        if (currentPort != null) {
            currentPort.close();
            currentPort = null;
            currentInputStream = null;
        }

        CommPortIdentifier cpi = CommPortIdentifier.getPortIdentifier(portId);
        currentPort = (SerialPort) cpi.open("MoppyFloppySim", 2000);
        currentInputStream = currentPort.getInputStream();
        currentPort.addEventListener(this);
        currentPort.notifyOnDataAvailable(true);
        currentPort.setSerialPortParams(9600, SerialPort.DATABITS_8,
                SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);
    }

    @Override
    public void serialEvent(SerialPortEvent spe) {
        switch (spe.getEventType()) {
            case SerialPortEvent.BI:
            case SerialPortEvent.OE:
            case SerialPortEvent.FE:
            case SerialPortEvent.PE:
            case SerialPortEvent.CD:
            case SerialPortEvent.CTS:
            case SerialPortEvent.DSR:
            case SerialPortEvent.RI:
            case SerialPortEvent.OUTPUT_BUFFER_EMPTY:
                break;
            case SerialPortEvent.DATA_AVAILABLE:
                byte[] readBuffer = new byte[20];

                try {
                    while (currentInputStream.available() > 2) {
                        int pin = currentInputStream.read();
                        byte[] payload = new byte[2];
                        currentInputStream.read(payload);

                        long loopPeriod = (payload[0] & 0xFF) << 8 | payload[1]
                                & 0xFF;
                        if (loopPeriod > 0) {
                            long microPeriod = loopPeriod
                                    * MoppyPlayerOutput.ARDUINO_RESOLUTION * 2; // The
                                                                                // actual
                                                                                // frequency
                                                                                // is
                                                                                // two
                                                                                // periods
                                                                                // (an
                                                                                // on
                                                                                // and
                                                                                // an
                                                                                // off)

                            double frequency = 1000000 / microPeriod;

                            fsw.displayNote(pin, String.valueOf(frequency)
                                    + "Hz");
                        } else
                            fsw.displayNote(pin, "");
                    }
                    System.out.print(new String(readBuffer));
                } catch (IOException e) {
                    System.out.println(e);
                }
                break;
        }
    }
}
