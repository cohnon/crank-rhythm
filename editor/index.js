const NoteType = Object.freeze({
  Normal: Symbol('type.normal'),
  Danger: Symbol('type.danger'),
  Click: Symbol('type.click'),
});

const NoteColor = Object.freeze({
  Black: Symbol('color.black'),
  White: Symbol('color.white'),
});

const NotePosition = Object.freeze({
  L1: Symbol('position.l1'),
  L2: Symbol('position.l2'),
  L3: Symbol('position.l3'),
  L4: Symbol('position.l4'),
  L5: Symbol('position.l5'),
  R1: Symbol('position.r1'),
  R2: Symbol('position.r2'),
  R3: Symbol('position.r3'),
  R4: Symbol('position.r4'),
  R5: Symbol('position.r5'),
});

class Note {
  constructor(type, color, position, beatTime) {
    this.type = type;
    this.color = color;
    this.position = position;
    this.beatTime = beatTime;
  }
}

const notes = [];
let notePicker = {
  type: NoteType.Normal,
  color: NoteColor.Black,
  position: NotePosition.L1,
};

var wavesurfer = WaveSurfer.create({
  container: '#waveform',
  waveColor: '#888563',
  progressColor: '#f0e65c',
  height: 80,
  scrollParent: true,
  minPxPerSec: 250,
  partialRender: true,
});

const loadButton = document.getElementById('load-btn');

loadButton.addEventListener('change', (e) => {
  var files = e.target.files;
  var file = URL.createObjectURL(files[0]);
  wavesurfer.load(file);
});

let playing = false;
const playButton = document.getElementById('play-btn');

const beatTime = document.getElementById('beat-time');
const time = document.getElementById('time');
const zoomSlider = document.getElementById('zoom-slider');
const speedSlider = document.getElementById('speed-slider');
let centerX = 0;
let centerY = 0;
let diskRadius = 0;

zoomSlider.addEventListener('change', (e) => {
  wavesurfer.zoom(zoomSlider.value);
});

speedSlider.addEventListener('change', (e) => {
  const wasPlaying = wavesurfer.isPlaying();

  wavesurfer.pause();
  wavesurfer.setPlaybackRate(speedSlider.value);

  if (wasPlaying) {
    wavesurfer.play();
  }
});

wavesurfer.on('audioprocess', () => {
  time.textContent = wavesurfer.getCurrentTime().toFixed(2);
  if (bpmInput.value.length > 0 && !isNaN(bpmInput.value)) {
    let offset = 0;
    if (offsetInput.value.length > 0 && !isNaN(offsetInput.value)) {
      offset = Number.parseFloat(offsetInput.value);
    }
    beatTime.textContent = ((wavesurfer.getCurrentTime() - offset) * Number.parseFloat(bpmInput.value) / 60).toFixed(2);
  }
});

wavesurfer.on('seek', () => {
  time.textContent = wavesurfer.getCurrentTime().toFixed(2);
  if (bpmInput.value.length > 0 && !isNaN(bpmInput.value)) {
    let offset = 0;
    if (offsetInput.value.length > 0 && !isNaN(offsetInput.value)) {
      offset = Number.parseFloat(offsetInput.value);
    }
    beatTime.textContent = ((wavesurfer.getCurrentTime() - offset) * Number.parseFloat(bpmInput.value) / 60).toFixed(2);
  }
});

playButton.addEventListener('click', onPlay);

function findNoteByBeatTime(noteBeatTime) {
  let front = 0;
  let back = notes.length - 1;
  while (front <= back) {
    const mid = Math.floor((front + back) / 2);
    if (notes[mid].beatTime > noteBeatTime) {
      back = mid - 1;
    } else if (notes[mid].beatTime < noteBeatTime) {
      front = mid + 1;
    } else {
      return { exists: true, index: mid };
    }
  }

  return { exists: false, index: front };
}

function insertNote(note) {
  const existingNote = findNoteByBeatTime(note.beatTime);
  if (existingNote.exists) {
    return;
  }

  notes.splice(existingNote.index, 0, note);
}

function removeNote(note) {
  const existingNote = findNoteByBeatTime(note.beatTime);
  if (existingNote.exists) {
    notes.splice(existingNote.index, 1);
  }
}

function updateNote(note, newNote) {
  removeNote(note);
  insertNote(newNote);
}

addEventListener('keydown', (e) => {
  if (typing) return;

  if (e.key === ' ') {
    onPlay();
  } else if (e.key === 'q') {
    const newNote = new Note(
      notePicker.type,
      notePicker.color,
      notePicker.position,
      Number.parseFloat(beatTime.textContent)
    );
    insertNote(newNote);
    selectedNote = newNote;
  } else if (e.key === 'w') {
    deleteNoteButton.click();
  } else if (e.key === 'a') {
    typeRadio[0].click();
  } else if (e.key === 's') {
    typeRadio[1].click();
  } else if (e.key === 'd') {
    typeRadio[2].click();
  } else if (e.key === 'z') {
    colorRadio[0].click();
  } else if (e.key === 'x') {
    colorRadio[1].click();
  }
});

function onPlay() {
  if (playing) {
    wavesurfer.pause();
    playButton.textContent = 'Play';
    playing = false;
  } else {
    wavesurfer.play();
    playing = true;
    playButton.textContent = 'Pause';
  }
}

// Controls
let selectedNote = null;
const deleteNoteButton = document.getElementById('delete-btn');
const typeRadio = document.querySelectorAll('input[name="note-type"]');
const colorRadio = document.querySelectorAll('input[name="note-color"]');
const positionRadio = document.querySelectorAll('input[name="note-pos"]');
colorRadio.forEach(radio => radio.addEventListener('change', () => {
  if (selectedNote) {
    selectedNote.color = NoteColor[radio.value];
  }
  notePicker.color = NoteColor[radio.value];
}));

typeRadio.forEach(radio => radio.addEventListener('change', () => {
  if (selectedNote) {
    selectedNote.type = NoteType[radio.value];
  }
  notePicker.type = NoteType[radio.value];
}));

positionRadio.forEach(radio => radio.addEventListener('change', () => {
  if (selectedNote) {
    selectedNote.position = NotePosition[radio.value];
  }
  notePicker.position = NotePosition[radio.value];
}));


deleteNoteButton.addEventListener('click', () => {
  if (selectedNote === null) return;

  removeNote(selectedNote);
});

// Preview
const canvas = document.getElementById('preview');
const context = canvas.getContext('2d');

const overlayCanvas = document.createElement('canvas');
const waveform = document.getElementById('waveform');
waveform.appendChild(overlayCanvas);
const overlayContext = overlayCanvas.getContext('2d');
overlayCanvas.id = 'overlay';

resize();

addEventListener('resize', e => {
  resize();
});

let clicking = false;
let clickingNote = false;
let clickPosX;
let clickTime;
overlayCanvas.addEventListener('mousedown', e => {
  const clickedBeatTime = ((e.clientX + waveform.firstChild.scrollLeft - 15) / wavesurfer.params.minPxPerSec - Number.parseFloat(offsetInput.value)) * Number.parseFloat(bpmInput.value) / 60;
  const existingNote = findNoteByBeatTime(clickedBeatTime);
  const rect = overlayCanvas.getBoundingClientRect();
  const mouseY = e.clientY - rect.top;
  if (notes[existingNote.index] && mouseY > 15 && mouseY < 50) {
    let diff = notes[existingNote.index].beatTime - clickedBeatTime;
    diff = diff * wavesurfer.params.minPxPerSec;
    if (diff < 50) {
      selectedNote = notes[existingNote.index];
      typeRadio.forEach(radio => { if (NoteType[radio.value] === selectedNote.type) radio.checked = true });
      colorRadio.forEach(radio => { if (NoteColor[radio.value] === selectedNote.color) radio.checked = true });
      positionRadio.forEach(radio => { if (NotePosition[radio.value] === selectedNote.position) radio.checked = true });
      clickingNote = true;
      return;
    }
  }

  clicking = true;
  clickPosX = e.clientX;
  clickTime = wavesurfer.getCurrentTime();
  clickMillis = Date.now();
});

overlayCanvas.addEventListener('mousemove', e => {
  if (clicking) {
    let time = (clickPosX - e.clientX) / wavesurfer.params.minPxPerSec + clickTime;
    let progress = time / wavesurfer.getDuration();
    if (progress < 0) progress = 0;
    if (progress > 1) progress = 1;
    wavesurfer.seekAndCenter(progress);
  } else if (clickingNote) {
    let noteTime = (e.clientX + waveform.firstChild.scrollLeft) / wavesurfer.params.minPxPerSec;
    let noteBeatTime = noteTime / 60 * Number.parseFloat(bpmInput.value);
    if (!findNoteByBeatTime(noteBeatTime).exists) {
      removeNote(selectedNote);
      selectedNote.beatTime = noteBeatTime;
      insertNote(selectedNote);
    }
  }
});

overlayCanvas.addEventListener('mouseleave', () => {clicking = false; clickingNote = false });
overlayCanvas.addEventListener('mouseup', e => {
  clicking = false;
  clickingNote = false;
  if (clickPosX === e.clientX) {
    selectedNote = null;
    let time = (e.clientX + waveform.firstChild.scrollLeft) / wavesurfer.params.minPxPerSec;
    let progress = time / wavesurfer.getDuration();
    wavesurfer.seekAndCenter(progress);
  }
});

let waveformScroll = 250;
overlayCanvas.addEventListener('wheel', e => {
  // waveformScroll += e.deltaY / 4;
  // if (waveformScroll < 1) waveformScroll = 1;
  // if (waveformScroll > 500) waveformScroll = 500;
  waveformScroll += e.deltaY / 4;
  zoomSlider.value = waveformScroll;
  wavesurfer.zoom(zoomSlider.value);
});


function drawDisk(t) {
  const rotation = t / 2000;
  const centerX = Math.floor(canvas.width / 2);
  const centerY = Math.floor(canvas.height / 2);
  let radius = Math.floor(Math.min(canvas.height / 8, canvas.width / 8));
  if (Number.parseFloat(beatTime.textContent) - Number.parseInt(beatTime.textContent) < 0.2) {
    radius += 3;
  }

  context.beginPath();
  context.arc(centerX, centerY, radius, 0, Math.PI * 2, false);
  context.fillStyle = '#b4acac';
  context.fill();

  context.beginPath();
  context.moveTo(centerX, centerY)
  context.arc(centerX, centerY, radius, rotation, (Math.PI / 2) + rotation, false);
  context.fillStyle = '#342c2c';
  context.fill();

  context.beginPath();
  context.moveTo(centerX, centerY);
  context.arc(centerX, centerY, radius, Math.PI + rotation, (Math.PI / 2 * 3) + rotation, false);
  context.fill();

  context.beginPath();
  context.arc(centerX, centerY, radius, 0, Math.PI * 2, false);
  context.strokeStyle = '#342c2c';
  context.lineWidth = 2;
  context.stroke();
}

function drawNotes() {
  const currentBeatTime = Number.parseFloat(beatTime.textContent);
  notes.forEach(note => {
    if (note.beatTime > currentBeatTime && note.beatTime + 2 > currentBeatTime) {
      let angle;
      switch (note.position) {
        case NotePosition.L1: angle = Math.PI / 4 * 3; break;
        case NotePosition.L2: angle = Math.PI / 8 * 7; break;
        case NotePosition.L3: angle = Math.PI; break;
        case NotePosition.L4: angle = Math.PI / 8 * 9; break;
        case NotePosition.L5: angle = Math.PI / 4 * 5; break;
        case NotePosition.R1: angle = Math.PI / 4; break;
        case NotePosition.R2: angle = Math.PI / 8; break;
        case NotePosition.R3: angle = 0; break;
        case NotePosition.R4: angle = Math.PI / 8 * 15; break;
        case NotePosition.R5: angle = Math.PI / 4 * 7; break;
      }
      const startX = centerX + Math.cos(angle) * centerX;
      const startY = centerY - Math.sin(angle) * centerX;
      const endX = centerX + Math.cos(angle) * diskRadius;
      const endY = centerY - Math.sin(angle) * diskRadius;
      const progress = 1 - (note.beatTime - currentBeatTime) / 2;
      const xPos = startX + progress * (endX - startX);
      const yPos = startY + progress * (endY - startY);
      drawNote(context, note, xPos, yPos);
    }
  });
}

function drawNote(ctx, note, xPos, yPos, selected = false) {

  if (note.type === NoteType.Danger) {
    ctx.beginPath();

    ctx.moveTo(xPos - 15, yPos - 15);
    ctx.lineTo(xPos + 15, yPos + 15);

    ctx.moveTo(xPos + 15, yPos - 15);
    ctx.lineTo(xPos - 15, yPos + 15);
    if (note.color === NoteColor.White) {
      ctx.lineWidth = 18;
      ctx.strokeStyle = '#342c2c';
      ctx.stroke();
    }
    ctx.lineWidth = 10;
    ctx.strokeStyle = note.color === NoteColor.Black ? '#342c2c' : '#b4acac'; 
    ctx.stroke();

    if (selected) {
      ctx.strokeStyle = '#f0f';
      ctx.lineWidth = 5;
      ctx.stroke();
    }
  } else {
    ctx.beginPath();
    ctx.arc(xPos, yPos, 15, 0, Math.PI * 2, false);
    if (note.color === NoteColor.White) {
      ctx.fillStyle = '#b4acac';
      ctx.fill();
      ctx.strokeStyle = '#342c2c';
      ctx.stroke();
    } else {
      ctx.fillStyle = '#342c2c';
      ctx.fill();
    }
  
    if (selected) {
      ctx.strokeStyle = '#f0f';
      ctx.lineWidth = 5;
      ctx.stroke();
    }

    if (note.type === NoteType.Click) {
      ctx.beginPath();
      ctx.arc(xPos, yPos, 20, 0, Math.PI * 2, false);
      ctx.strokeStyle = '#342c2c';
      ctx.lineWidth = 4;
      ctx.stroke();
    }
    
  }
}

function drawOverlay() {
  overlayContext.fillStyle = '#f00'
  notes.forEach(note => {
    let xPos = (note.beatTime * 60 / Number.parseFloat(bpmInput.value) + Number.parseFloat(offsetInput.value)) * wavesurfer.params.minPxPerSec;
    xPos -= waveform.firstChild.scrollLeft;
    const yPos = 36;

    drawNote(overlayContext, note, xPos, yPos, selectedNote && selectedNote.beatTime === note.beatTime);
  });
}

function resize() {
  canvas.width = canvas.clientWidth;
  canvas.height = canvas.clientHeight;
  overlayCanvas.width = overlayCanvas.clientWidth;
  overlayCanvas.height = overlayCanvas.clientHeight;

  centerX = Math.floor(canvas.width / 2);
  centerY = Math.floor(canvas.height / 2);
  diskRadius = Math.floor(Math.min(canvas.width / 8, canvas.height / 8));
}

requestAnimationFrame(() => updateLoop(0));
function updateLoop(t) {
  context.fillStyle = '#b4acac';
  context.fillRect(0, 0, canvas.width, canvas.height);
  drawNotes();
  drawDisk(t);

  overlayContext.clearRect(0, 0, overlayCanvas.width, overlayCanvas.height);
  drawOverlay();

  requestAnimationFrame(updateLoop);
}

// Save .txt
let typing = false;
const saveButton = document.getElementById('save-btn');

saveButton.addEventListener('click', saveBeatmap);

const songNameInput = document.getElementById('name-input');
const bpmInput = document.getElementById('bpm-input');
const offsetInput = document.getElementById('offset-input');
songNameInput.addEventListener('focus', () => typing = true);
bpmInput.addEventListener('focus', () => typing = true);
offsetInput.addEventListener('focus', () => typing = true);
songNameInput.addEventListener('blur', () => typing = false);
bpmInput.addEventListener('blur', () => typing = false);
offsetInput.addEventListener('blur', () => typing = false);

function saveBeatmap() {
  let data =
    '0\n' +
    songNameInput.value + '\n' + 
    bpmInput.value + '\n' + 
    offsetInput.value + '\n';
  
  notes.forEach(note => {
    let typeId, colorId, posId, beatTime;
    switch (note.type) {
      case NoteType.Normal: typeId = 0; break;
      case NoteType.Click: typeId = 1; break;
      case NoteType.Danger: typeId = 2; break;
    }

    switch (note.color) {
      case NoteColor.Black: colorId = 0; break;
      case NoteColor.White: colorId = 1; break;
    }

    switch (note.position) {
      case NotePosition.L1: posId = 0; break;
      case NotePosition.L2: posId = 1; break;
      case NotePosition.L3: posId = 2; break;
      case NotePosition.L4: posId = 3; break;
      case NotePosition.L5: posId = 4; break;
      case NotePosition.R1: posId = 5; break;
      case NotePosition.R2: posId = 6; break;
      case NotePosition.R3: posId = 7; break;
      case NotePosition.R4: posId = 8; break;
      case NotePosition.R5: posId = 9; break;
    }

    beatTime = note.beatTime.toFixed(3);

    data += `${typeId} ${colorId} ${posId} ${beatTime}\n`;
  });
        
  // Convert the text to BLOB.
  const textToBLOB = new Blob([data], { type: 'text/plain' });
  const sFileName = 'beatmap.txt';	   // The file to save the data.

  let newLink = document.createElement("a");
  newLink.download = sFileName;

  if (window.webkitURL != null) {
    newLink.href = window.webkitURL.createObjectURL(textToBLOB);
  }
  else {
    newLink.href = window.URL.createObjectURL(textToBLOB);
    newLink.style.display = "none";
    document.body.appendChild(newLink);
  }

  newLink.click(); 

}

// BPM Test
let clicks = -1;
let startTime = 0;
let lastClick = 0;

const bpmButton = document.getElementById('bpm-btn');
const bpmDisplay = document.getElementById('bpm-display');
let clearBpmTimeout = null;

bpmButton.addEventListener('click', () => {
  lastClick = Date.now();
  clicks += 1;
  clearTimeout(clearBpmTimeout);
  clearBpmTimeout = setTimeout(() => {
    clicks = -1;
    bpmDisplay.textContent = 0;
  }, 2000);

  if (clicks === 0) {
    startTime = Date.now();
  } else {
    bpmDisplay.textContent = (clicks / (lastClick - startTime) * 60000).toFixed(0);
  }
});