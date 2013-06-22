/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package electronicsclub.moppyproject.testing;

import gnu.io.CommPortIdentifier;

import java.util.ArrayList;
import java.util.Enumeration;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.jdesktop.application.SingleFrameApplication;

/**
 * 
 * @author Graham Campbell
 * @author Michael Marley
 * @author SammyIAm
 * 
 */
public class MoppyFloppySim extends SingleFrameApplication {

    MoppyCOMsumer mCom;

    /**
     * Main method launching the application.
     */
    public static void main(String[] args) {
        launch(MoppyFloppySim.class, args);
    }

    @Override
    protected void startup() {
        FloppySimWindow mainWindow = new FloppySimWindow(this);
        mCom = new MoppyCOMsumer(mainWindow);
        show(mainWindow);
        try {
            mCom.openPort(mainWindow.getSelectedCOM()); // Try first port.
        } catch (Exception ex) {
            Logger.getLogger(MoppyFloppySim.class.getName()).log(Level.SEVERE,
                    null, ex);
        }

        new Thread(mCom).start();
    }

    public static String[] getAvailableCOMPorts() {
        ArrayList<String> portIdentifierStrings = new ArrayList<String>();

        Enumeration<CommPortIdentifier> comPorts = CommPortIdentifier
                .getPortIdentifiers();
        while (comPorts.hasMoreElements())
            portIdentifierStrings.add(comPorts.nextElement().getName());

        return portIdentifierStrings.toArray(new String[portIdentifierStrings
                .size()]);
    }
}
