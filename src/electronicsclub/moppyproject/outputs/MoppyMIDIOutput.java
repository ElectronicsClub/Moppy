/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package electronicsclub.moppyproject.outputs;

import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.MidiDevice;
import javax.sound.midi.MidiDevice.Info;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.MidiSystem;
import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.Receiver;
import javax.sound.midi.ShortMessage;

/**
 * 
 * @author Graham Campbell
 * @author Michael Marley
 * @author SammyIAm
 * 
 */
public class MoppyMIDIOutput implements MoppyReceiver {

    MidiDevice device;
    Receiver deviceReceiver;

    public MoppyMIDIOutput(String midiDeviceName)
            throws MidiUnavailableException {
        MidiDevice.Info[] infos = MidiSystem.getMidiDeviceInfo();
        for (Info i : infos)
            try {
                if (i.getName().equalsIgnoreCase(midiDeviceName))
                    device = MidiSystem.getMidiDevice(i);
            } catch (MidiUnavailableException ex) {
                Logger.getLogger(MoppyMIDIOutput.class.getName()).log(
                        Level.SEVERE, null, ex);
            }

        device.open();
        deviceReceiver = device.getReceiver();
    }

    public static HashMap<String, Info> getMIDIOutInfos() {
        MidiDevice.Info[] infos = MidiSystem.getMidiDeviceInfo();
        HashMap<String, Info> outInfos = new HashMap<String, Info>();

        for (Info i : infos)
            try {
                MidiDevice dev = MidiSystem.getMidiDevice(i);
                if (dev.getMaxReceivers() != 0)
                    outInfos.put(i.getName(), i);
            } catch (MidiUnavailableException ex) {
                Logger.getLogger(MoppyMIDIOutput.class.getName()).log(
                        Level.SEVERE, null, ex);
            }
        return outInfos;
    }

    @Override
    public void send(MidiMessage message, long timeStamp) {
        deviceReceiver.send(message, timeStamp);
    }

    @Override
    public void close() {
        deviceReceiver.close();
        device.close();
    }

    @Override
    public void reset() {
        // Nothing really to do here, I don't think.
        if (deviceReceiver != null)
            try {
                ShortMessage resetMessage = new ShortMessage();
                resetMessage.setMessage(ShortMessage.SYSTEM_RESET);
                deviceReceiver.send(resetMessage, -1);
            } catch (InvalidMidiDataException ex) {
                Logger.getLogger(MoppyMIDIOutput.class.getName()).log(
                        Level.SEVERE, null, ex);
            }
    }
}
