
import {instantiateStreaming} from "assemblyscript/lib/loader";
export default instantiateStreaming(
    fetch('./hello.wasm')
).then(rawModule => Object.assign({}, rawModule, {
    scramble: input => {
        const pInput = rawModule.__retain(rawModule.__allocString(input));

        const pOutput = rawModule.scramble(pInput);

        const result = rawModule.__getString(pOutput);
        rawModule.__release(pInput);
        rawModule.__release(pOutput);
        return result;
    }
}));
// https://github.com/DevAndyLee/webassembly-in-react/blob/master/src/wa-api.js