let times = [];
let running = false;
let start_time;
let bpm;
let spb;
const bpm_input = document.getElementById("bpm-input");
const output = document.getElementById("output");

addEventListener("keydown", (e) => {
  if (e.key === " ") {
    if (!running) {
      const song = document.getElementById("song");
      bpm = Number.parseFloat(bpm_input.value);
      spb = 60 / bpm;
      running = true;
      start_time = Date.now();
      song.play();
      return;  
    }

    times.push((Date.now() - start_time) / 1000 * bpm / 60);
    // 170 b/m * 1m/60s = 170/60 b/s
  }

  if (e.key === "p") {
    times.forEach(time => {
      output.value += Math.floor(time) + "\n";
    });
    times = [];
    running = false;
  }
});
