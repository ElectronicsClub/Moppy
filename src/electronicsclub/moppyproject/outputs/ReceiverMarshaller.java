package electronicsclub.moppyproject.outputs;

import java.util.ArrayList;
import java.util.Arrays;

import javax.sound.midi.MidiDevice;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.Receiver;
import javax.sound.midi.ShortMessage;

/**
 * 
 * @author Graham Campbell
 * @author Michael Marley
 * @author SammyIAm
 * 
 */
public class ReceiverMarshaller implements MoppyReceiver {

    /**
     * Array of {@link MoppyReceiver} references, one for each channel. The same
     * receiver object can be assigned to multiple indexes if it is going to be
     * handling multiple channels of data.
     */
    private final MoppyReceiver[] outputReceivers = new MoppyReceiver[16];

    /**
     * Creates a new ReceiverMarshaller with an empty array of {@link Receiver}
     * s.
     * 
     * @param receivers
     */
    public ReceiverMarshaller() {
        // Nothing for now.
    }

    /**
     * Sets a channel's Receiver object. If a receiver is already assigned to
     * this channel, {@link Receiver#close()} will be called before it is
     * removed and replaced with the new {@link Receiver}.
     * 
     * @param MIDIChannel
     * @param channelReceiver
     */
    public void setReceiver(int MIDIChannel, MoppyReceiver channelReceiver) {
        if (MIDIChannel < 1 || MIDIChannel > 16)
            throw new IllegalArgumentException(
                    "Only channels 1-16 are supported by the ReceiverMarshaller!");
        if (outputReceivers[MIDIChannel - 1] != null)
            outputReceivers[MIDIChannel - 1].close();
        outputReceivers[MIDIChannel - 1] = channelReceiver;
    }

    /**
     * Closes all receivers, and removes them from the array (fills array with
     * nulls).
     */
    public void clearReceivers() {
        close();
    }

    @Override
    public void send(MidiMessage message, long timeStamp) {
        int ch = ((ShortMessage) message).getChannel();
        if (outputReceivers[ch] != null)
            outputReceivers[ch].send(message, timeStamp);
    }

    /**
     * Explicity closes all output receivers, and nulls the array. This differs
     * slightly from the use described in {@link MidiDevice} in that the
     * ReceiverMarshaller itself is not neccesarily closed when this is called.
     * After being closed, new receivers can continue to be added to the
     * ReceiverMarshaller.
     */
    @Override
    public void close() {
        for (Receiver r : outputReceivers)
            if (r != null)
                r.close();
        Arrays.fill(outputReceivers, null);
    }

    /**
     * Finds the unique set of receivers and calls the
     * {@link MoppyReceiver#reset() } method. We go through the trouble of
     * finding uniques incase the reset is time-consuming.
     */
    @Override
    public void reset() {
        ArrayList<MoppyReceiver> uniqueReceivers = new ArrayList<MoppyReceiver>();
        for (MoppyReceiver r : outputReceivers)
            if (r != null && !uniqueReceivers.contains(r))
                uniqueReceivers.add(r);
        for (MoppyReceiver r : uniqueReceivers)
            r.reset();
    }
}
