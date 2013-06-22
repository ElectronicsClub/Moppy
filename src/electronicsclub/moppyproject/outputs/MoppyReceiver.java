package electronicsclub.moppyproject.outputs;

import javax.sound.midi.Receiver;

/**
 * 
 * @author Graham Campbell
 * @author Michael Marley
 * @author SammyIAm
 * 
 */
public interface MoppyReceiver extends Receiver {

    /**
     * Returns the drives/xylophone/calliope/organ/drums to a reset-state. This
     * should not disconnect or dispose of any connection though.
     */
    public void reset();
}
