param([string[]]$Names = @())

$ErrorActionPreference = 'Stop'
$repoPath = Split-Path -Parent $PSScriptRoot
New-Item -ItemType Directory -Path (Join-Path $repoPath 'build') -Force | Out-Null
Add-Type -AssemblyName System.Speech
$synth = New-Object System.Speech.Synthesis.SpeechSynthesizer
$synth.SelectVoice('Microsoft Katja')
$prompts = @(
  @{ Folder = 'mp3'; Name = '0982_modifier_night_light'; Text = 'Nachtlicht. Die LEDs leuchten gelb mit der eingestellten Nachtlichthelligkeit. Wähle anschließend die Dauer oder unbegrenzt.' },
  @{ Folder = 'mp3'; Name = '1100_modifier_quiet_light'; Text = 'Ruhelicht. Die LEDs leuchten schwach und gleichmäßig. Musik und Einschlaftimer bleiben aktiv.' },
  @{ Folder = 'mp3'; Name = '1101_modifier_next_animation'; Text = 'Wiedergabeanimation wechseln. Jedes Auflegen dieser Karte wählt die nächste Animation. Die Auswahl bleibt nach dem Neustart gespeichert.' },
  @{ Folder = 'mp3'; Name = '1102_modifier_timed_night'; Text = 'Nachtlicht mit wählbarer Dauer. Nach Ablauf wird nur das Licht ausgeschaltet. Du kannst auch unbegrenzt wählen.' },
  @{ Folder = 'mp3'; Name = '1103_night_duration_intro'; Text = 'Wie lange soll das Nachtlicht leuchten? Dreißig Minuten sind voreingestellt. Mit Hoch und Runter kannst du die Zeit ändern. Nach sechzig Minuten folgt unbegrenzt. Bestätige mit der Pausetaste.' },
  @{ Folder = 'mp3'; Name = '1104_modifier_next_color'; Text = 'Grundfarben durchschalten. Jedes erneute Auflegen wählt die nächste von acht Farben. Die Auswahl bleibt gespeichert.' },
  @{ Folder = 'mp3'; Name = '1105_modifier_fixed_color'; Text = 'Eine feste Grundfarbe setzen. Wähle beim Erstellen die Farbe für diese Karte. Beim Auflegen wird immer diese Farbe eingestellt.' },
  @{ Folder = 'mp3'; Name = '1106_card_color_intro'; Text = 'Welche Farbe soll auf die Karte? Mit Hoch und Runter kannst du die Farbe auf dem Streifen ansehen. Die Pausetaste bestätigt. Die Grundfarbe der Box wird erst beim Auflegen der fertigen Karte geändert.' },
  @{ Folder = 'mp3'; Name = '1107_night_duration_unlimited'; Text = 'Unbegrenzt.' },
  @{ Folder = 'advert'; Name = '0351_quiet_light'; Text = 'Ruhelicht aktiviert.' },
  @{ Folder = 'advert'; Name = '0352_next_animation'; Text = 'Nächste Wiedergabeanimation gewählt.' },
  @{ Folder = 'advert'; Name = '0353_timed_night_light'; Text = 'Nachtlicht mit Zeitbegrenzung aktiviert.' },
  @{ Folder = 'advert'; Name = '0354_color_selected'; Text = 'Grundfarbe eingestellt.' }
)
try {
  foreach ($prompt in $prompts) {
    if ($Names.Count -gt 0 -and $prompt.Name -notin $Names) { continue }
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
