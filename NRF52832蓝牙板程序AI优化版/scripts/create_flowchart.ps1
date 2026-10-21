# Rebuild the standalone SVG and PNG using the same nodes and edges.
# System.Drawing is available in Windows PowerShell; no downloads are required.
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$root = Split-Path -Parent $PSScriptRoot
$nodes = New-Object System.Collections.ArrayList
$edges = New-Object System.Collections.ArrayList
function Node($x, $y, $w, $h, $kind, $label, $fill) {
    [void]$nodes.Add(@{X=$x;Y=$y;W=$w;H=$h;Kind=$kind;Label=$label;Fill=$fill})
}
function Edge($points, $label = '', $lx = 0, $ly = 0) {
    [void]$edges.Add(@{Points=$points;Label=$label;X=$lx;Y=$ly})
}

Node 330 112 260 56 'rect' '上电 / 复位' '#dbeafe'
Node 170 205 580 92 'rect' "初始化 SoftDevice → 启动被动扫描`n初始化串口 / TIMER1 / PPI / LED`n输出启动信息" '#dbeafe'
Node 330 352 260 118 'diamond' '已收到开始命令？' '#fef3c7'
Node 70 524 340 76 'rect' "配置阶段`n丢弃已入队广播" '#e0f2fe'
Node 525 524 360 76 'rect' "运行阶段`n取一条广播报告" '#dcfce7'
Node 70 656 340 88 'rect' "读取串口数据`n先复制，再重启接收`n按 CRLF 拼接命令行" '#e0f2fe'
Node 70 794 340 124 'diamond' "命令完整且`n参数有效？" '#fef3c7'
Node 70 968 340 88 'rect' "保存配置并应答`n开始命令会锁定配置`n再次配置需复位" '#e0f2fe'
Node 505 654 400 132 'diamond' "有报告且 RSSI 达标？`n自定义模式还需`n特征前缀匹配" '#fef3c7'
Node 525 832 360 64 'rect' '按设置输出原始地址和 DATA' '#dcfce7'
Node 525 942 360 96 'rect' "XOR：校验 → 解码 MAC / SN`n明文：提取 16 字节唯一码`n和 7 字节 SN" '#dcfce7'
Node 525 1084 360 80 'rect' "输出有效解码字段`n无法解码也结束当前行" '#dcfce7'
Node 170 1220 650 78 'rect' "维护接收后更新 LED 心跳，进入下一次循环`n运行阶段仅丢弃串口数据，不接受新配置" '#dbeafe'

Node 1003 205 392 80 'rect' "BLE 回调收到广播`n先检查指针和长度（≤31 字节）" '#ede9fe'
Node 1003 332 392 116 'diamond' "报告合法且`n队列未满？" '#fef3c7'
Node 1020 490 360 72 'rect' "复制报告后发布写索引`n等待主循环取走" '#ede9fe'
Node 1003 595 392 76 'rect' "不满足：丢弃新报告`n队列容量 127 条，不覆盖旧报告" '#f1f5f9'
Node 1003 778 392 70 'rect' "串口 EasyDMA 接收`n每字节触发 PPI 重置空闲计时" '#ede9fe'
Node 1003 885 392 84 'rect' "2 ms 空闲 / 收满 / 硬件错误`n异步请求停止 RX`n回调发布长度或错误标志" '#ede9fe'
Node 1003 1005 392 90 'rect' "RXTO → PPI 2 → TIMER1 CC1`n确认硬件已停止`n等待主循环复制并重启接收" '#ede9fe'
Node 1003 1138 392 86 'rect' "回调不打印、不忙等`nTX 使用驱动自有 RAM 缓冲`n打印采用有限等待" '#f1f5f9'

Edge @(@(460,168),@(460,205))
Edge @(@(460,297),@(460,352))
Edge @(@(330,411),@(240,411),@(240,524)) '否' 282 422
Edge @(@(590,411),@(705,411),@(705,524)) '是' 652 422
Edge @(@(240,600),@(240,656))
Edge @(@(240,744),@(240,794))
Edge @(@(240,918),@(240,968)) '是' 261 933
Edge @(@(70,856),@(49,856),@(49,1189),@(495,1189),@(495,1220))
Edge @(@(240,1056),@(240,1189),@(495,1189),@(495,1220))
Edge @(@(705,600),@(705,654))
Edge @(@(705,786),@(705,832)) '是' 726 798
Edge @(@(905,720),@(943,720),@(943,1189),@(495,1189),@(495,1220)) '否' 925 735
Edge @(@(705,896),@(705,942))
Edge @(@(705,1038),@(705,1084))
Edge @(@(705,1164),@(705,1189),@(495,1189),@(495,1220))
Edge @(@(495,1298),@(495,1340),@(26,1340),@(26,322),@(460,322),@(460,352))
Edge @(@(1199,285),@(1199,332))
Edge @(@(1199,448),@(1199,490)) '是' 1220 460
Edge @(@(1395,390),@(1420,390),@(1420,633),@(1395,633)) '否' 1410 437
Edge @(@(1199,848),@(1199,885))
Edge @(@(1199,969),@(1199,1005))

$bitmap = New-Object System.Drawing.Bitmap 1440,1440
$graphics = [Drawing.Graphics]::FromImage($bitmap)
$graphics.SmoothingMode = [Drawing.Drawing2D.SmoothingMode]::AntiAlias
$graphics.TextRenderingHint = [Drawing.Text.TextRenderingHint]::AntiAliasGridFit
$graphics.Clear([Drawing.Color]::White)
$font = New-Object Drawing.Font('Microsoft YaHei',18,[Drawing.FontStyle]::Regular,[Drawing.GraphicsUnit]::Pixel)
$titleFont = New-Object Drawing.Font('Microsoft YaHei',30,[Drawing.FontStyle]::Bold,[Drawing.GraphicsUnit]::Pixel)
$headingFont = New-Object Drawing.Font('Microsoft YaHei',22,[Drawing.FontStyle]::Bold,[Drawing.GraphicsUnit]::Pixel)
$textBrush = New-Object Drawing.SolidBrush([Drawing.ColorTranslator]::FromHtml('#16324f'))
$linePen = New-Object Drawing.Pen([Drawing.ColorTranslator]::FromHtml('#64748b'),2)
$arrowPen = New-Object Drawing.Pen([Drawing.ColorTranslator]::FromHtml('#64748b'),2)
$arrowPen.EndCap = [Drawing.Drawing2D.LineCap]::ArrowAnchor
$format = New-Object Drawing.StringFormat
$format.Alignment = [Drawing.StringAlignment]::Center
$format.LineAlignment = [Drawing.StringAlignment]::Center
$svg = New-Object Text.StringBuilder
[void]$svg.AppendLine('<svg xmlns="http://www.w3.org/2000/svg" width="1440" height="1440" viewBox="0 0 1440 1440" role="img" aria-labelledby="title desc">')
[void]$svg.AppendLine('<title id="title">NRF52832 蓝牙广播接收程序流程图</title><desc id="desc">左侧为配置和运行阶段的主循环，右侧为 BLE 与 UART 异步事件。有效命令更新配置；运行时按 RSSI 和可选特征筛选，解码并串口输出。</desc>')
[void]$svg.AppendLine('<defs><marker id="arrow" markerWidth="8" markerHeight="8" refX="7" refY="4" orient="auto"><path d="M0,0 L8,4 L0,8" fill="#64748b"/></marker></defs><rect width="1440" height="1440" fill="white"/>')
[void]$svg.AppendLine('<style>text{font-family:"Microsoft YaHei","Noto Sans CJK SC",sans-serif;fill:#16324f;font-size:18px} .heading{font-size:22px;font-weight:bold}</style>')

function Label($label, $x, $y, $width, $height, $labelFont, $css = '') {
    $rect = New-Object Drawing.RectangleF($x,$y,$width,$height)
    $graphics.DrawString($label,$labelFont,$textBrush,$rect,$format)
    $lines = $label.Split("`n")
    $spacing = if ($css -eq 'title') { 38 } else { 25 }
    $firstY = $y + $height/2 - (($lines.Count-1)*$spacing)/2 + 6
    $centerX = $x + $width/2
    $style = if ($css -eq 'title') { ' style="font-size:30px;font-weight:bold"' } elseif ($css -eq 'heading') { ' class="heading"' } else { '' }
    [void]$svg.AppendLine("<text x=`"$centerX`" y=`"$firstY`" text-anchor=`"middle`"$style>")
    for ($i=0; $i -lt $lines.Count; ++$i) {
        $escaped = [Security.SecurityElement]::Escape($lines[$i])
        $lineY = $firstY + $i*$spacing
        [void]$svg.AppendLine("<tspan x=`"$centerX`" y=`"$lineY`">$escaped</tspan>")
    }
    [void]$svg.AppendLine('</text>')
}
Label 'NRF52832 蓝牙广播接收程序 · 总流程' 40 20 1360 45 $titleFont 'title'
Label '先配置，再持续过滤与输出；BLE 和 UART 事件与主循环并行。' 40 68 1360 30 $font
Label '主循环' 50 148 220 32 $headingFont 'heading'
Label 'BLE 异步事件' 1003 143 392 38 $headingFont 'heading'
Label 'UART 异步事件' 1003 715 392 38 $headingFont 'heading'

foreach ($edge in $edges) {
    $coords = @($edge.Points | ForEach-Object { "$( $_[0]),$( $_[1])" }) -join ' '
    [void]$svg.AppendLine("<polyline points=`"$coords`" fill=`"none`" stroke=`"#64748b`" stroke-width=`"2`" marker-end=`"url(#arrow)`"/>")
    for ($i=1; $i -lt $edge.Points.Count; ++$i) {
        $pen = if ($i -eq $edge.Points.Count-1) { $arrowPen } else { $linePen }
        $graphics.DrawLine($pen,[single]$edge.Points[$i-1][0],[single]$edge.Points[$i-1][1],[single]$edge.Points[$i][0],[single]$edge.Points[$i][1])
    }
}
foreach ($node in $nodes) {
    $x=$node.X; $y=$node.Y; $w=$node.W; $h=$node.H
    $brush = New-Object Drawing.SolidBrush([Drawing.ColorTranslator]::FromHtml($node.Fill))
    if ($node.Kind -eq 'diamond') {
        [Drawing.PointF[]]$points = @([Drawing.PointF]::new($x+$w/2,$y),[Drawing.PointF]::new($x+$w,$y+$h/2),[Drawing.PointF]::new($x+$w/2,$y+$h),[Drawing.PointF]::new($x,$y+$h/2))
        $graphics.FillPolygon($brush,$points); $graphics.DrawPolygon($linePen,$points)
        $coords = "$($x+$w/2),$y $($x+$w),$($y+$h/2) $($x+$w/2),$($y+$h) $x,$($y+$h/2)"
        [void]$svg.AppendLine("<polygon points=`"$coords`" fill=`"$($node.Fill)`" stroke=`"#64748b`"/>")
    } else {
        $graphics.FillRectangle($brush,$x,$y,$w,$h); $graphics.DrawRectangle($linePen,$x,$y,$w,$h)
        [void]$svg.AppendLine("<rect x=`"$x`" y=`"$y`" width=`"$w`" height=`"$h`" rx=`"8`" fill=`"$($node.Fill)`" stroke=`"#64748b`"/>")
    }
    Label $node.Label $x $y $w $h $font
    $brush.Dispose()
}
foreach ($edge in $edges) { if ($edge.Label) { Label $edge.Label ($edge.X-15) $edge.Y 30 24 $font } }
Label '否 / 未完整' 63 927 130 28 $font
Label '自定义：RSSI + 特征前缀；默认：仅 RSSI。两种解码都要求 31 字节输入。' 60 1370 1320 30 $font
[void]$svg.AppendLine('</svg>')
[IO.File]::WriteAllText((Join-Path $root '0_readme/程序流程图.svg'),$svg.ToString(),[Text.UTF8Encoding]::new($false))
$bitmap.Save((Join-Path $root '0_readme/程序流程图.png'),[Drawing.Imaging.ImageFormat]::Png)
$format.Dispose(); $font.Dispose(); $titleFont.Dispose(); $headingFont.Dispose(); $textBrush.Dispose(); $linePen.Dispose(); $arrowPen.Dispose(); $graphics.Dispose(); $bitmap.Dispose()
Write-Output 'Created 0_readme SVG and PNG flowcharts.'
