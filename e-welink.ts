const MICROBIT_ID_EWELINK = 2301;
const MICROBIT_EWELINK_EVT_ON_WRITTEN = 60;

enum EWeLinkDirection {
    //% block="stop"
    Stop,
    //% block="left"
    Left,
    //% block="right"
    Right,
    //% block="up"
    Up,
    //% block="down"
    Down,
}

/**
 * Supports for eWeLink service.
 */
//% weight=50 icon="\uf1eb" color=#1976d2
namespace eWeLink {
    let onPressHandlers: { [direction: string]: () => void } = {};
    let pressDirection: EWeLinkDirection = EWeLinkDirection.Stop;

    /**
     * Starts service.
     */
    //% blockId=ewelink_start block="eWeLink start"
    //% weight=90 blockGap=8
    export function start(): void {
        startService();

        // on received
        control.onEvent(MICROBIT_ID_EWELINK, MICROBIT_EWELINK_EVT_ON_WRITTEN, () => {
            const buffer = readBuffer();
            if (buffer.length > 0) {
                const data = buffer.toArray(NumberFormat.UInt8LE);
                const sum = data.reduce((previous, current) => previous + current, 0);
                if (2 * data[data.length - 1] === sum) {
                    const cmd: number = data[4];
                    pressDirection = cmd;
                    onPressHandlers[cmd] && onPressHandlers[cmd]();
                }
            }
        });
    }

    /**
     * On eWeLink controller pressed.
     */
    //% blockId=ewelink_onpressed
    //% block="on eWeLink|%direction pressed"
    //% weight=70 blockGap=8
    export function onPressed(direction: EWeLinkDirection, handler: () => void) {
        onPressHandlers[direction] = handler;
    }

    /**
     * Get eWeLink controller press state.
     */
    //% blockId=ewelink_ispressed
    //% block="eWeLink|%direction is pressed"
    //% weight=60 blockGap=32
    export function isPressed(direction: EWeLinkDirection) {
        return direction === pressDirection;
    }

    /**
     * Returns true if the service is connected.
     */
    //% blockId=ewelink_isconnected block="eWeLink is connected"
    //% weight=20 blockGap=8 shim=eWeLink::isConnected
    export function isConnected(): boolean {
        return false;
    }

    /**
     * Sets the bluetooth transmit power between 0 (minimal) and 7 (maximum).
     * @param power power level between 0 (minimal) and 7 (maximum), eg: 7.
     */
    //% weight=10
    //% blockId=ewelink_settransmitpower block="eWeLink|set transmit power %power"
    //% power.min=0 power.max=7 shim=eWeLink::setTransmitPower
    //% blockGap=8
    export function setTransmitPower(power: number) {
        return;
    }

    /**
     * Starts the eWeLink service.
     */
    //% shim=eWeLink::startService
    export function startService(): void {
        return;
    }

    /**
     * read buffer.
     */
    //% shim=eWeLink::readBuffer
    export function readBuffer(): Buffer {
        return Buffer.create(0);
    }
}
