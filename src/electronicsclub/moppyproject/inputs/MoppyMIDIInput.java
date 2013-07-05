package electronicsclub.moppyproject.inputs;

import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.sound.midi.MidiDevice;
import javax.sound.midi.MidiDevice.Info;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.MidiSystem;
import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.Receiver;
import javax.sound.midi.Transmitter;

/**
 * 
 * @author Graham Campbell
 * @author Michael Marley
 * @author SammyIAm
 * 
 */
public class MoppyMIDIInput implements Transmitter, Receiver {
    MidiDevice currentDevice = null;
    Receiver downstreamReceiver = null;

    public void setDevice(Info deviceInfo) throws MidiUnavailableException {
        if (currentDevice != null)
            currentDevice.close();
        currentDevice = MidiSystem.getMidiDevice(deviceInfo);
        currentDevice.open();
        currentDevice.getTransmitter().setReceiver(this);
    }

    @Override
    public void setReceiver(Receiver receiver) {
        downstreamReceiver = receiver;
    }

    @Override
    public Receiver getReceiver() {
        return downstreamReceiver;
    }

    @Override
    public void close() {
        if (currentDevice != null) {
            currentDevice.close();
            currentDevice = null;
        }
        if (downstreamReceiver != null) {
            downstreamReceiver.close();
            downstreamReceiver = null;
        }
    }

    //
    // / Receiver logic for forwarding messages
    //

    @Override
    public void send(MidiMessage message, long timeStamp) {
        if (downstreamReceiver != null)
            downstreamReceiver.send(message, timeStamp);
    }

    //
    // // UTILITY METHODS
    //

    public static HashMap<String, MidiDevice.Info> getMIDIInInfos() {
        MidiDevice.Info[] infos = MidiSystem.getMidiDeviceInfo();
        HashMap<String, MidiDevice.Info> outInfos = new HashMap<String, MidiDevice.Info>();

        for (MidiDevice.Info i : infos)
            try {
                MidiDevice dev = MidiSystem.getMidiDevice(i);
                if (dev.getMaxTransmitters() != 0)
                    outInfos.put(i.getName(), i);
            } catch (MidiUnavailableException ex) {
                Logger.getLogger(MoppyMIDIInput.class.getName()).log(
                        Level.SEVERE, null, ex);
            }
        return outInfos;
    }
}
