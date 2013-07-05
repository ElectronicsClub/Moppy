/*
 * To change this template, choose Tools | Templates and open the template in
 * the editor.
 */
package electronicsclub.moppyproject;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.prefs.BackingStoreException;
import java.util.prefs.Preferences;

import javax.swing.JOptionPane;

import org.jdesktop.application.SingleFrameApplication;

import electronicsclub.moppyproject.inputs.MoppyMIDIInput;
import electronicsclub.moppyproject.inputs.MoppySequencer;
import electronicsclub.moppyproject.outputs.ReceiverMarshaller;
import electronicsclub.moppyproject.ui.MoppyControlWindow;

/**
 * 
 * @author Graham Campbell
 * @author Michael Marley
 * @author SammyIAm
 * 
 */
public class MoppyUI extends SingleFrameApplication {

    // Input objects
    public MoppySequencer ms;
    public MoppyMIDIInput midiIn;
    /**
     * The {@link ReceiverMarshaller} will be added as a receiver to whatver
     * input object is selected.
     */
    public ReceiverMarshaller rm = new ReceiverMarshaller();
    public Preferences prefs = Preferences.userNodeForPackage(MoppyUI.class);

    @Override
    protected void startup() {
        // Initialize parts
        try {
            ms = new MoppySequencer();
            midiIn = new MoppyMIDIInput();

        } catch (Exception ex) {
            JOptionPane.showMessageDialog(getMainFrame(), ex.toString());
        }
        MoppyControlWindow mainWindow = new MoppyControlWindow(this);
        mainWindow.setStatus("Initializing...");
        show(mainWindow);
        mainWindow.setStatus("Initialized.");
    }

    /**
     * Main method launching the application.
     */
    public static void main(String[] args) {
        launch(MoppyUI.class, args);
    }

    public void savePreferences() {
        try {
            prefs.flush();
        } catch (BackingStoreException ex) {
            Logger.getLogger(MoppyUI.class.getName()).log(Level.SEVERE, null,
                    ex);
        }
    }

    public void putPreferenceObject(String key, Object object) {
        try {
            prefs.putByteArray(key, serializePref(object));
        } catch (IOException ex) {
            Logger.getLogger(MoppyUI.class.getName()).log(Level.SEVERE, null,
                    ex);
        }
    }

    public Object getPreferenceObject(String key) {
        try {
            return deserializePref(prefs.getByteArray(key, null));
        } catch (NullPointerException ex) {
            Logger.getLogger(MoppyUI.class.getName()).log(Level.WARNING,
                    "No preference set for " + key, ex);
        } catch (IOException ex) {
            Logger.getLogger(MoppyUI.class.getName()).log(Level.SEVERE, null,
                    ex);
        } catch (ClassNotFoundException ex) {
            Logger.getLogger(MoppyUI.class.getName()).log(Level.SEVERE, null,
                    ex);
        }

        return null;
    }

    // Preference to object
    private static Object deserializePref(byte[] b)
            throws NullPointerException, IOException, ClassNotFoundException {
        return new ObjectInputStream(new ByteArrayInputStream(b)).readObject();
    }

    // Object to preference
    private static byte[] serializePref(Object p) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        ObjectOutputStream oos = new ObjectOutputStream(baos);
        oos.writeObject(p);
        oos.close();
        return baos.toByteArray();
    }
}
