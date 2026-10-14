$ErrorActionPreference = 'Stop'
$repoPath = Split-Path -Parent $PSScriptRoot
New-Item -ItemType Directory -Path (Join-Path $repoPath 'build') -Force | Out-Null
Add-Type -AssemblyName System.Speech
$synth = New-Object System.Speech.Synthesis.SpeechSynthesizer
$synth.SelectVoice('Microsoft Katja')
$prompts = @(
  @{ Folder = 'mp3'; Name = '1100_modifier_quiet_light'; Text = 'Ruhelicht. Die LEDs leuchten schwach und gleichmäßig. Musik und Einschlaftimer bleiben aktiv.' },
  @{ Folder = 'mp3'; Name = '1101_modifier_lively_light'; Text = 'Lichtprofil Lebhaft. Zwei Lichtpunkte wandern mit wechselnden Farben über den Streifen.' },
  @{ Folder = 'mp3'; Name = '1102_modifier_timed_night'; Text = 'Nachtlicht mit Zeitbegrenzung. Am Ende wird nur das Licht ausgeschaltet.' },
  @{ Folder = 'mp3'; Name = '1103_night_duration_intro'; Text = 'Wie lange soll das Nachtlicht leuchten? Dreißig Minuten sind voreingestellt. Mit Hoch und Runter kannst du die Zeit ändern. Bestätige mit der Pausetaste.' },
  @{ Folder = 'advert'; Name = '0351_quiet_light'; Text = 'Ruhelicht aktiviert.' },
  @{ Folder = 'advert'; Name = '0352_lively_light'; Text = 'Lichtprofil Lebhaft aktiviert.' },
  @{ Folder = 'advert'; Name = '0353_timed_night_light'; Text = 'Nachtlicht mit Zeitbegrenzung aktiviert.' }
)
try {
  foreach ($prompt in $prompts) {
    $wavPath = Join-Path $repoPath ('build\' + $prompt.Name + '.wav')
    $mp3Path = Join-Path $repoPath ('sd-card\' + $prompt.Folder + '\' + $prompt.Name + '.mp3')
    $synth.SetOutputToWaveFile($wavPath)
    $synth.Speak($prompt.Text)
    $synth.SetOutputToNull()
    & ffmpeg -hide_banner -loglevel error -y -i $wavPath -ac 1 -ar 22050 -codec:a libmp3lame -b:a 64k $mp3Path
    if ($LASTEXITCODE -ne 0) { throw "Audio conversion failed: $($prompt.Name)" }
    Get-Item -LiteralPath $mp3Path | Select-Object Name,Length
  }
} finally { $synth.Dispose() }
