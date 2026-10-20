// Run after web/build.sh and npm ci --prefix web. Starts its own temporary server.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const http = require('node:http');
const { chromium } = require('playwright');
const root = path.resolve(__dirname, '../build-web');
const results = path.resolve(__dirname, 'test-results');
const types = { '.html': 'text/html', '.js': 'text/javascript', '.wasm': 'application/wasm', '.data': 'application/octet-stream' };
const server = http.createServer((req, res) => {
  const name = req.url === '/' ? 'index.html' : req.url.slice(1);
  if (!['index.html', 'index.js', 'index.wasm', 'index.data'].includes(name)) { res.writeHead(404).end(); return; }
  res.setHeader('Content-Type', types[path.extname(name)]);
  const stream = fs.createReadStream(path.join(root, name));
  stream.on('error', () => res.writeHead(404).end());
  stream.pipe(res);
});
async function frames(page, count = 20) {
  const start = await page.evaluate(() => Module.frames || 0);
  await page.waitForFunction(n => Module.failure || Module.frames >= n, start + count);
  assert.equal(await page.evaluate(() => Module.failure), undefined);
}
async function state(page, expected) {
  await page.waitForFunction(n => Module.failure || Module.gameState === n, expected);
  assert.equal(await page.evaluate(() => Module.failure), undefined);
  await frames(page);
}
async function pixels(page) {
  return page.evaluate(() => new Promise(resolve => requestAnimationFrame(() => {
    const c = Module.canvas, gl = c.getContext('webgl2');
    const data = new Uint8Array(c.width * c.height * 4);
    gl.readPixels(0, 0, c.width, c.height, gl.RGBA, gl.UNSIGNED_BYTE, data);
    const colours = new Set(); let total = 0, hash = 2166136261;
    for (let i = 0; i < data.length; i += 4 * 113) {
      colours.add(`${data[i]},${data[i + 1]},${data[i + 2]}`);
      total += data[i] + data[i + 1] + data[i + 2];
      hash = Math.imul(hash ^ (data[i] + data[i + 1] * 256 + data[i + 2] * 65536), 16777619);
    }
    resolve({ colours: colours.size, brightness: total, hash, error: gl.getError() });
  })));
}
(async () => {
  let browser;
  try {
    fs.mkdirSync(results, { recursive: true });
    await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
    const executablePath = process.env.CHROMIUM_PATH || (fs.existsSync('/usr/bin/chromium') ? '/usr/bin/chromium' : undefined);
    browser = await chromium.launch({ executablePath, headless: true,
      args: ['--no-sandbox', '--use-gl=angle', '--use-angle=swiftshader', '--enable-unsafe-swiftshader'] });
    const page = await browser.newPage({ viewport: { width: 1120, height: 1000 } });
    const errors = [];
    page.on('pageerror', e => errors.push(e.message));
    page.on('console', m => { if (m.type() === 'error') errors.push(m.text()); });
    const url = `http://127.0.0.1:${server.address().port}`;
    await page.goto(url);
    await state(page, 2);
    let rendered = await pixels(page);
    assert.ok(rendered.colours > 8, 'title must contain rendered geometry');
    assert.equal(rendered.error, 0);
    assert.equal(await page.evaluate(() => Module.audio.bank.size), 14);
    await page.screenshot({ path: path.join(results, 'title.png') });
    await page.keyboard.press('Enter'); // Intentionally a tap shorter than one frame.
    await state(page, 3);
    await page.screenshot({ path: path.join(results, 'landscape.png') });
    await page.keyboard.press('Enter');
    await state(page, 4);
    await frames(page, 35); // Complete the half-second fade-in.
    rendered = await pixels(page);
    assert.ok(rendered.colours > 12, 'game must contain terrain, objects and the energy panel');
    assert.equal(rendered.error, 0);
    await page.screenshot({ path: path.join(results, 'game.png') });
    await page.keyboard.press('ArrowLeft');
    await frames(page, 10);
    assert.notEqual((await pixels(page)).hash, rendered.hash, 'turning must change the rendered view');
    await page.keyboard.press('p'); await frames(page, 40);
    const paused = await pixels(page);
    await page.keyboard.press('p'); await frames(page, 40);
    assert.ok((await pixels(page)).brightness > paused.brightness * 1.5, 'resume must remove pause dimming');
    await page.locator('#play').click();
    await page.waitForFunction(() => document.pointerLockElement?.id === 'canvas');
    const beforeMouse = await pixels(page);
    await page.mouse.move(650, 350); await frames(page, 10);
    assert.notEqual((await pixels(page)).hash, beforeMouse.hash, 'pointer-lock mouse look must change the view');
    await page.evaluate(() => document.exitPointerLock());
    assert.equal(await page.evaluate(() => Module.audio.context.state), 'running');
    assert.ok(await page.evaluate(() => {
      const data = Module.audio.bank.get('title.wav').getChannelData(0);
      return data.some(n => Math.abs(n) > .01);
    }), 'decoded title audio must contain samples');
    await page.keyboard.press('m'); await frames(page, 5);
    const saved = await page.evaluate(() => localStorage.getItem('augmentinel.settings'));
    assert.match(saved, /MusicEnabled\t0/);
    assert.match(saved, /LastLandscape\t0000/);
    await page.keyboard.press('Escape'); await state(page, 3);
    assert.ok((await pixels(page)).colours > 8, 'returning to landscape selection must render terrain');
    await page.setViewportSize({ width: 800, height: 900 }); await frames(page, 5);
    assert.ok((await pixels(page)).colours > 8, 'resizing must retain the scene');
    await page.reload(); await state(page, 2);
    assert.equal(await page.evaluate(() => localStorage.getItem('augmentinel.settings')), saved);
    assert.deepEqual(errors, [], 'browser must have no JS or WebGL errors');
    console.log('PASS: WASM startup, assets, rendering, title → landscape → game, quick taps, turning, pause/resume, mouse look, Web Audio, resize, and saved settings after reload.');
  } finally {
    if (browser) await browser.close();
    await new Promise(resolve => server.close(resolve));
  }
})().catch(e => { console.error(e); process.exitCode = 1; });
