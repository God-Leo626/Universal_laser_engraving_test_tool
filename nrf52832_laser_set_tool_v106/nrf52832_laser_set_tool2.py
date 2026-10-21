# -*- coding: utf-8 -*-
import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import time
import threading
import json
import os
import sys
import serial
from serial.tools import list_ports

FONT_FAMILY = "Microsoft YaHei"
BG_COLOR = "#1E1E1E"
FG_COLOR = "#F5F5F5"
BTN_BG = "#333333"
BTN_FG = "#FFFFFF"
ENTRY_BG = "#2D2D2D"

TREE_HOVER_BG = "#2060C0"
TREE_SELECT_BG = "#009933"
TREE_NORMAL_BG = "#D4A017"
TREE_NORMAL_FG = "#111111"

VERSION = "V1.0.6"


class SerialCmdTool:
    def __init__(self, root):
        self.root = root
        self.root.title(f"多功能串口调试工具 {VERSION} | HEX+字符自动校验+自动回复")
        self.root.geometry("1600x880")
        self.root.configure(bg=BG_COLOR)

        # ========== 设置窗口ICON(sonoff.ico) ==========
        if getattr(sys, 'frozen', False):
            self.app_path = os.path.dirname(sys.executable)
        else:
            self.app_path = os.path.dirname(os.path.abspath(__file__))
        self.save_file = os.path.join(self.app_path, "cmd_save.json")
        self.note_save_file = os.path.join(self.app_path, "note_save.json")
        self.log_txt_path = os.path.join(self.app_path, "log.txt")
        self.ico_path = os.path.join(self.app_path, "sonoff.ico")
        try:
            if os.path.exists(self.ico_path):
                self.root.iconbitmap(self.ico_path)
        except Exception:
            pass

        self.baudrate = 115200
        self.bytesize = 8
        self.parity = "N"
        self.stopbits = 1

        self.ser = None
        self.is_open = False
        self.is_sending = False
        self.read_running = False
        self.read_thread = None

        self.lock_cmd = tk.BooleanVar(value=False)
        self.send_interval = tk.IntVar(value=500)

        self.enable_auto_reply = tk.BooleanVar(value=False)
        self.reply_mode = tk.StringVar(value="hex")
        self.reply_full_preview = tk.StringVar(value="7F 07 01 02 | XOR:XX")
        self.reply_trigger_lock = False

        self.cmd_list = []
        self.log_hex_mode = True
        self.log_cache = []

        self.note_text_content = ""

        # 日志折叠状态
        self.log_panel_shown = tk.BooleanVar(value=True)

        self.set_style()
        self.build_menu()
        self.build_ui()
        # 初始化布局后刷新一次，确保尺寸计算准确
        self.root.update_idletasks()
        self.load_auto_cmds()
        self.load_note_data()
        self.refresh_com()

    def save_rx_to_log(self, recv_bytes):
        if not self.enable_auto_reply.get():
            return
        try:
            time_str = time.strftime("%Y-%m-%d %H:%M:%S")
            hex_str = " ".join([f"{b:02X}" for b in recv_bytes])
            line = f"[{time_str}] RX DATA: {hex_str}\n"
            with open(self.log_txt_path, "a", encoding="utf-8") as f:
                f.write(line)
        except Exception:
            pass

    def build_menu(self):
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)
        help_menu = tk.Menu(menubar, tearoff=0)
        help_menu.add_command(label="使用说明", command=self.show_help_dialog)
        menubar.add_cascade(label="Help", menu=help_menu)
        note_menu = tk.Menu(menubar, tearoff=0)
        note_menu.add_command(label="打开文本记录", command=self.open_note_window)
        menubar.add_cascade(label="记录", menu=note_menu)

    def show_help_dialog(self):
        help_win = tk.Toplevel(self.root)
        help_win.title("工具使用说明")
        help_win.geometry("720x520")
        help_win.configure(bg=BG_COLOR)
        help_win.transient(self.root)
        help_win.grab_set()
        # 弹窗居中主窗口
        self.root.update_idletasks()
        main_x = self.root.winfo_x()
        main_y = self.root.winfo_y()
        main_w = self.root.winfo_width()
        main_h = self.root.winfo_height()
        win_w = 720
        win_h = 520
        pos_x = main_x + (main_w - win_w) // 2
        pos_y = main_y + (main_h - win_h) // 2
        help_win.geometry(f"{win_w}x{win_h}+{pos_x}+{pos_y}")

        help_text = scrolledtext.ScrolledText(help_win, bg="#111111", fg="#00FF00", font=(FONT_FAMILY, 10))
        help_text.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)
        help_content = """【多功能串口调试工具 V1.3.0】
串口固定参数：波特率115200，数据位8，无校验，停止位1 (8N1)

====================
1、两种命令模式
【十六进制命令】
帧结构：前缀(1字节) + 长度(1字节) + 载荷 + XOR校验字节
校验范围：长度字节 + 载荷，异或结果作为最后一个字节。

【字符串命令】
输入字符串，发送时自动追加 \\r\\n 换行。

2、命令列表操作
- 添加命令：填写参数后点添加
- ↑↓按钮：选中条目，上下移动调整发送顺序
- 双击列表行：修改已有命令
- 每行前方复选框：勾选代表批量发送时生效，不勾选跳过
- 每行后方【发送】按钮：单独发送本条命令
- 锁定命令：勾选后禁止新增/删除/修改命令，防止误操作
- 关闭软件自动保存命令列表，下次打开自动加载

3、批量发送
设置发送间隔(ms)，点击【开始逐条发送】，仅发送勾选的命令；可随时停止。

4、自动回复功能
开启后，串口收到数据，会自动发送配置的回复帧，连续发送2次，间隔1秒；
发送完成后等待下一次接收再触发回复。
字符串模式自动追加\\r\\n。

5、日志
日志支持HEX/字符两种显示模式，支持清除日志；中间<< >>按钮可折叠/展开日志面板。

6、【记录菜单】
打开文本记录窗口，可以记录调试笔记；关闭窗口/关闭主程序自动保存，下次打开自动载入。

7、新增日志保存功能
- 勾选【开启收到数据自动回复】后，自动记录所有接收数据
- 日志保存在工具同级目录 log.txt
- 每一次接收数据单独占用一行，持续追加不覆盖

8、十进制转十六进制小工具
命令编辑区右侧输入十进制数字（支持带符号负数，8位补码转换），自动转换显示十六进制。

9、注意事项
- 先选择串口，打开串口之后才能发送数据
- 打包exe后，cmd_save.json、note_save.json、log.txt会生成在exe同目录
"""
        help_text.insert(tk.END, help_content)
        help_text.config(state=tk.DISABLED)

    def open_note_window(self):
        note_win = tk.Toplevel(self.root)
        note_win.title("文本记录笔记")
        note_win.geometry("700x480")
        note_win.configure(bg=BG_COLOR)
        note_win.transient(self.root)
        # 弹窗居中主窗口
        self.root.update_idletasks()
        main_x = self.root.winfo_x()
        main_y = self.root.winfo_y()
        main_w = self.root.winfo_width()
        main_h = self.root.winfo_height()
        win_w = 700
        win_h = 480
        pos_x = main_x + (main_w - win_w) // 2
        pos_y = main_y + (main_h - win_h) // 2
        note_win.geometry(f"{win_w}x{win_h}+{pos_x}+{pos_y}")

        note_win.grab_set()
        lbl = ttk.Label(note_win, text="在此记录调试文本，关闭窗口/关闭主程序自动保存")
        lbl.pack(pady=4)
        self.note_widget = scrolledtext.ScrolledText(note_win, bg="#111111", fg="#EEEEEE", font=(FONT_FAMILY,10))
        # 修改光标颜色为红色
        self.note_widget.config(insertbackground="#FF0000")
        self.note_widget.pack(fill=tk.BOTH, expand=True, padx=8, pady=4)
        self.note_widget.insert(tk.END, self.note_text_content)
        def on_note_close():
            self.note_text_content = self.note_widget.get("1.0", tk.END)
            self.save_note_data()
            note_win.destroy()
        note_win.protocol("WM_DELETE_WINDOW", on_note_close)

    def load_note_data(self):
        if not os.path.exists(self.note_save_file):
            return
        try:
            with open(self.note_save_file, "r", encoding="utf-8") as f:
                js = json.load(f)
                self.note_text_content = js.get("note", "")
        except Exception:
            self.note_text_content = ""

    def save_note_data(self):
        try:
            with open(self.note_save_file, "w", encoding="utf-8") as f:
                json.dump({"note": self.note_text_content}, f, ensure_ascii=False, indent=2)
        except Exception:
            pass

    def set_style(self):
        style = ttk.Style()
        style.theme_use("clam")
        style.configure(".", font=(FONT_FAMILY, 9), background=BG_COLOR, foreground=FG_COLOR)
        style.configure("TFrame", background=BG_COLOR)
        style.configure("TLabelframe", background=BG_COLOR, foreground=FG_COLOR)
        style.configure("TLabelframe.Label", background=BG_COLOR, foreground=FG_COLOR, font=(FONT_FAMILY, 10))
        style.configure("TLabel", background=BG_COLOR, foreground=FG_COLOR, font=(FONT_FAMILY, 9))
        style.configure("TEntry", fieldbackground=ENTRY_BG, foreground=FG_COLOR)
        style.configure("TRadiobutton", background=BG_COLOR, foreground="#0088ff")
        style.configure("TCheckbutton", background=BG_COLOR, foreground="#0088ff")
        style.configure("TButton", background=BTN_BG, foreground=BTN_FG)
        style.map("TButton", background=[("active", "#444444")])
        style.configure("Treeview.Heading", background="#D4A017", foreground="#111111")
        self.tree_tag_normal = "normal_row"
        self.tree_tag_hover = "hover"
        self.tree_tag_select = "selected_row"

    def toggle_log_panel(self):
        """切换日志面板显示/隐藏，隐藏后窗口自动收窄无黑边，左侧区域尺寸保持不变"""
        if self.log_panel_shown.get():
            # 先刷新布局获取准确尺寸
            self.root.update_idletasks()
            win_height = self.root.winfo_height()
            # 计算左侧区域 + 中间按钮 + 主容器内边距的总宽度
            left_width = self.left_frame.winfo_width()
            mid_width = self.mid_frame.winfo_width()
            # 主容器左右padding各10，合计20
            new_win_width = left_width + mid_width + 20

            # 隐藏日志面板
            self.right_frame.grid_remove()
            self.btn_toggle_log.config(text=">>")
            self.log_panel_shown.set(False)

            # 调整窗口宽度，刚好容纳左侧内容，无多余黑边
            self.root.geometry(f"{new_win_width}x{win_height}")
        else:
            win_height = self.root.winfo_height()
            # 恢复日志面板显示
            self.right_frame.grid()
            self.btn_toggle_log.config(text="<<")
            self.log_panel_shown.set(True)

            # 恢复初始窗口宽度
            self.root.geometry(f"1600x{win_height}")

    def _dec_to_hex_convert(self, event=None):
        """十进制输入框，支持带符号十进制，转为8位补码HEX"""
        try:
            dec_str = self.var_dec_in.get().strip()
            if not dec_str:
                self.var_hex_out.set("")
                return
            dec_val = int(dec_str)
            # 8位有符号数转补码：范围 -128 ~ 127 对应 00 ~ FF
            u8_val = dec_val & 0xFF
            self.var_hex_out.set(f"{u8_val:02X}")
        except ValueError:
            self.var_hex_out.set("输入非十进制数字")

    def build_ui(self):
        self.main = ttk.Frame(self.root, padding=10)
        self.main.pack(fill=tk.BOTH, expand=True)
        # =========设置主grid权重=========
        self.main.grid_rowconfigure(0, weight=1)
        self.main.grid_columnconfigure(0, weight=3)
        self.main.grid_columnconfigure(1, weight=0)
        self.main.grid_columnconfigure(2, weight=2)

        self.left_frame = ttk.Frame(self.main)
        self.left_frame.grid(row=0, column=0, sticky="nsew", padx=(0,0))

        # 中间切换按钮
        self.mid_frame = ttk.Frame(self.main)
        self.mid_frame.grid(row=0, column=1, sticky="ns")
        self.btn_toggle_log = tk.Button(self.mid_frame, text="<<", width=2, font=(FONT_FAMILY,11,"bold"),
                                         bg="#444444", fg="#00FF00", command=self.toggle_log_panel)
        self.btn_toggle_log.pack(expand=True, fill=tk.BOTH)

        self.right_frame = ttk.LabelFrame(self.main, text="收发日志")
        self.right_frame.grid(row=0, column=2, sticky="nsew", padx=(10,0))

        top_bar = ttk.Frame(self.left_frame)
        top_bar.pack(fill=tk.X, pady=4)
        ttk.Label(top_bar, text="SONOFF", font=(FONT_FAMILY, 18, "bold"), foreground="#D4A017", background=BG_COLOR).pack(side=tk.LEFT, padx=4)
        ttk.Label(top_bar, text=f"多功能串口调试工具 {VERSION}", font=(FONT_FAMILY, 12), foreground=FG_COLOR, background=BG_COLOR).pack(side=tk.LEFT, padx=12)

        com_frame = ttk.LabelFrame(self.left_frame, text="串口设置 (115200 8N1)")
        com_frame.pack(fill=tk.X, pady=4)
        ttk.Label(com_frame, text="串口号：").grid(row=0, column=0, padx=4, pady=6)
        self.com_cb = ttk.Combobox(com_frame, width=15, state="readonly")
        self.com_cb.option_add("*TCombobox*Listbox.background", "#333333")
        # ========= 修改：下拉列表文字颜色改为蓝色 #0088FF =========
        self.com_cb.option_add("*TCombobox*Listbox.foreground", "#0088FF")
        self.com_cb.option_add("*TCombobox*Listbox.selectBackground", TREE_SELECT_BG)
        self.com_cb.option_add("*TCombobox*Listbox.selectForeground", "#FFFFFF")
        self.com_cb.option_add("*TCombobox*Listbox.activeBackground", TREE_HOVER_BG)
        self.com_cb.option_add("*TCombobox*Listbox.activeForeground", "#FFFFFF")
        self.com_cb.grid(row=0, column=1, padx=4, pady=6)
        ttk.Button(com_frame, text="刷新串口", command=self.refresh_com).grid(row=0, column=2, padx=4)
        self.btn_com = tk.Button(com_frame, text="打开串口", width=12, font=(FONT_FAMILY, 9, "bold"), bg="#880000", fg="white", command=self.toggle_com)
        self.btn_com.grid(row=0, column=3, padx=6)
        ttk.Label(com_frame, text="发送间隔(ms)：").grid(row=0, column=4, padx=8)
        ttk.Entry(com_frame, textvariable=self.send_interval, width=8).grid(row=0, column=5)
        ttk.Checkbutton(com_frame, text="锁定命令(防误改)", variable=self.lock_cmd, command=self.lock_change).grid(row=0, column=6, padx=10)

        reply_frame = ttk.LabelFrame(self.left_frame, text="自动回复设置｜收到数据触发，间隔1s回复2次后停止，等待下次接收再触发")
        reply_frame.pack(fill=tk.X, pady=4)
        reply_frame.grid_columnconfigure(6, weight=1)
        ttk.Checkbutton(reply_frame, text="开启收到数据自动回复", variable=self.enable_auto_reply).grid(row=0, column=0, padx=6, pady=4)
        ttk.Radiobutton(reply_frame, text="回复十六进制", variable=self.reply_mode, value="hex", command=self.on_reply_mode_change).grid(row=0, column=1, padx=6)
        ttk.Radiobutton(reply_frame, text="回复字符串(自动\\r\\n)", variable=self.reply_mode, value="str", command=self.on_reply_mode_change).grid(row=0, column=2, padx=6)
        ttk.Label(reply_frame, text="HEX载荷内容：").grid(row=0, column=3, padx=6)
        self.ent_reply_content = ttk.Entry(reply_frame, width=32)
        self.ent_reply_content.insert(0, "88 80 00 00")
        self.ent_reply_content.grid(row=0, column=4, padx=4, pady=4)
        self.ent_reply_content.bind("<KeyRelease>", self.calc_reply_xor_preview)
        ttk.Label(reply_frame, text="完整帧预览：").grid(row=0, column=5, padx=8)
        ttk.Entry(reply_frame, textvariable=self.reply_full_preview, state="readonly").grid(row=0, column=6, padx=4, pady=4, sticky="ew")

        edit_frame = ttk.LabelFrame(self.left_frame, text="命令编辑")
        edit_frame.pack(fill=tk.X, pady=6)
        edit_frame.grid_columnconfigure(5, weight=1)
        self.cmd_mode = tk.StringVar(value="hex")
        ttk.Radiobutton(edit_frame, text="十六进制命令", variable=self.cmd_mode, value="hex", command=self.switch_mode).grid(row=0, column=0, padx=2, pady=4)
        ttk.Radiobutton(edit_frame, text="字符串命令(自动\\r\\n)", variable=self.cmd_mode, value="str", command=self.switch_mode).grid(row=0, column=1, padx=2, pady=4)
        ttk.Label(edit_frame, text="命令含义备注：").grid(row=0, column=2, padx=4, pady=4, sticky="w")
        self.ent_desc = ttk.Entry(edit_frame)
        self.ent_desc.insert(0, "")
        self.ent_desc.grid(row=0, column=3, columnspan=3, padx=4, pady=4, sticky="ew")
        ttk.Label(edit_frame, text="固定前缀(1字节HEX)：").grid(row=1, column=0, pady=4, sticky="w")
        self.ent_prefix = ttk.Entry(edit_frame)
        self.ent_prefix.insert(0, "7F")
        self.ent_prefix.grid(row=1, column=1, padx=4, pady=4, sticky="ew")
        ttk.Label(edit_frame, text="命令总长度(十进制)：").grid(row=1, column=2, pady=4, sticky="w")
        self.ent_len = ttk.Entry(edit_frame)
        self.ent_len.insert(0, "7")
        self.ent_len.grid(row=1, column=3, padx=4, pady=4, sticky="ew")
        ttk.Label(edit_frame, text="HEX载荷：").grid(row=1, column=4, pady=4, sticky="w")
        self.ent_payload = ttk.Entry(edit_frame)
        self.ent_payload.insert(0, "01 02 03")
        self.ent_payload.grid(row=1, column=5, padx=4, pady=4, sticky="ew")
        ttk.Label(edit_frame, text="字符串内容：").grid(row=2, column=0, pady=4, sticky="w")
        self.ent_str = ttk.Entry(edit_frame)
        self.ent_str.insert(0, "mxy")
        self.ent_str.grid(row=2, column=1, columnspan=5, padx=4, pady=4, sticky="ew")
        btn_f = ttk.Frame(edit_frame)
        btn_f.grid(row=3, column=0, sticky="w", columnspan=6, padx=2, pady=8)
        btn_width = 12
        ttk.Button(btn_f, text="添加命令", width=btn_width, command=self.add_cmd).grid(row=0, column=0, padx=2)
        ttk.Button(btn_f, text="删除选中", width=btn_width, command=self.del_cmd).grid(row=0, column=1, padx=2)
        ttk.Button(btn_f, text="清空列表", width=btn_width, command=self.clear_cmd).grid(row=0, column=2, padx=2)
        ttk.Button(btn_f, text="手动保存", width=btn_width, command=self.save_cmds).grid(row=0, column=3, padx=2)

        # =========十进制转HEX输入区域（红框位置）=========
        self.var_dec_in = tk.StringVar()
        self.var_hex_out = tk.StringVar()
        ttk.Label(btn_f, text="DEC->HEX:").grid(row=0, column=4, padx=(12,2))
        ent_dec = ttk.Entry(btn_f, textvariable=self.var_dec_in, width=10)
        ent_dec.grid(row=0, column=5, padx=2)
        ent_dec.bind("<KeyRelease>", self._dec_to_hex_convert)
        ent_hex_show = ttk.Entry(btn_f, textvariable=self.var_hex_out, state="readonly", width=10)
        ent_hex_show.grid(row=0, column=6, padx=2)

        list_frame = ttk.LabelFrame(self.left_frame, text="命令列表（可调整顺序 ↑↓，双击行修改命令及含义；勾选=批量发送生效；发送按钮单独发送本条）")
        # =========修复空白：fill+expand，吃掉剩余垂直空间，消除底部黑边=========
        list_frame.pack(fill=tk.BOTH, expand=True, pady=4)
        list_con = ttk.Frame(list_frame)
        list_con.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)
        # 新增列：enable(勾选), mode, desc, data
        self.tree = ttk.Treeview(list_con, columns=("enable","mode", "desc", "data"), show="headings")
        self.tree.heading("enable", text="启用")
        self.tree.heading("mode", text="类型")
        self.tree.heading("desc", text="命令含义")
        self.tree.heading("data", text="命令内容")
        self.tree.column("enable", width=60, anchor="center")
        self.tree.column("mode", width=90)
        self.tree.column("desc", width=240)
        self.tree.column("data", width=640)
        self.tree.tag_configure(self.tree_tag_normal, background=TREE_NORMAL_BG, foreground=TREE_NORMAL_FG)
        self.tree.tag_configure(self.tree_tag_hover, background=TREE_HOVER_BG, foreground="#FFFFFF")
        self.tree.tag_configure(self.tree_tag_select, background=TREE_SELECT_BG, foreground="#FFFFFF")
        self.tree.tag_bind(self.tree_tag_select, "<<TreeviewSelect>>", lambda e: None)
        self.tree.bind("<<TreeviewSelect>>", self._on_tree_select_change)
        self.tree.bind("<Double-1>", self.on_tree_double_click)
        # 单击第一列切换勾选状态
        self.tree.bind("<Button-1>", self.on_tree_click_checkbox)

        scroll = ttk.Scrollbar(list_con, orient=tk.VERTICAL, command=self.tree.yview)
        self.tree.configure(yscrollcommand=scroll.set)
        self.tree.grid(row=0, column=0, sticky="nsew")
        scroll.grid(row=0, column=1, sticky="ns")

        move_f = ttk.Frame(list_con)
        move_f.grid(row=0, column=2, padx=4)
        ttk.Button(move_f, text="↑上移", command=self.move_up).pack(pady=4)
        ttk.Button(move_f, text="↓下移", command=self.move_down).pack(pady=4)
        # 单独发送本条按钮
        ttk.Button(move_f, text="发送本条", command=self.send_selected_one).pack(pady=4)

        list_con.grid_rowconfigure(0, weight=1)
        list_con.grid_columnconfigure(0, weight=1)

        ctrl_frame = ttk.Frame(self.left_frame)
        ctrl_frame.pack(fill=tk.X, pady=6)
        ttk.Button(ctrl_frame, text="▶ 开始逐条发送", command=self.start_send).pack(side=tk.LEFT, padx=5)
        ttk.Button(ctrl_frame, text="■ 停止发送", command=self.stop_send).pack(side=tk.LEFT, padx=5)
        self.btn_log_mode = ttk.Button(ctrl_frame, text="日志:HEX模式", command=self.toggle_log_display_mode)
        self.btn_log_mode.pack(side=tk.LEFT, padx=5)
        ttk.Button(ctrl_frame, text="清除日志", command=self.clear_log).pack(side=tk.RIGHT, padx=5)

        # 右侧日志面板
        log_toolbar = ttk.Frame(self.right_frame)
        log_toolbar.pack(fill=tk.X, padx=4, pady=2)
        self.log = scrolledtext.ScrolledText(self.right_frame, bg="#111111", fg="#00FF00", font=(FONT_FAMILY, 9))
        self.log.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

        self.switch_mode()
        self.calc_reply_xor_preview()

    def on_tree_click_checkbox(self, event):
        region = self.tree.identify("region", event.x, event.y)
        if region != "cell":
            return
        col = self.tree.identify_column(event.x)
        # #1 对应第一列 enable
        if col == "#1":
            item = self.tree.identify_row(event.y)
            if not item:
                return
            idx = self.tree.index(item)
            d = self.cmd_list[idx]
            d["enable"] = not d.get("enable", True)
            self.refresh_tree()
            self.save_cmds()
            # 关键修复：点击勾选列，阻止事件继续传递，不会变更选中行
            return "break"

    def send_selected_one(self):
        if not self.is_open or not self.ser or not self.ser.is_open:
            messagebox.showwarning("提示", "请先打开串口")
            return
        sel = self.tree.selection()
        if not sel:
            messagebox.showinfo("提示", "请选中一条命令")
            return
        idx = self.tree.index(sel[0])
        item = self.cmd_list[idx]
        self.ser.write(item["raw"])
        self.log_tx_bytes(item["raw"], item["show"])

    def _on_tree_select_change(self, event):
        # 先全部重置为普通标签
        for item in self.tree.get_children():
            self.tree.item(item, tags=(self.tree_tag_normal,))
        sel = self.tree.selection()
        for it in sel:
            self.tree.item(it, tags=(self.tree_tag_select,))

    def on_reply_mode_change(self, *args):
        self.calc_reply_xor_preview()

    def calc_reply_xor_preview(self, event=None):
        if self.reply_mode.get() != "hex":
            self.reply_full_preview.set("字符串模式，自动追加\\r\\n")
            return
        try:
            payload = self.hex2bytes(self.ent_reply_content.get())
            head = b"\x7F\x07"
            check_segment = b"\x07" + payload
            xor_val = self.xor_check(check_segment)
            full_frame = head + payload + bytes([xor_val])
            show_str = " ".join(f"{b:02X}" for b in full_frame)
            self.reply_full_preview.set(f"{show_str}")
        except Exception:
            self.reply_full_preview.set("解析错误")

    def build_auto_reply_bytes(self):
        mode = self.reply_mode.get()
        content_text = self.ent_reply_content.get()
        if mode == "hex":
            payload = self.hex2bytes(content_text)
            head = b"\x7F\x07"
            check_segment = b"\x07" + payload
            xor_byte = bytes([self.xor_check(check_segment)])
            full_frame = head + payload + xor_byte
            return full_frame
        else:
            # 输入原样，底层追加真实\r\n
            return content_text.encode("utf-8") + b"\r\n"

    def reply_worker_two_times(self, reply_frame:bytes):
        try:
            for i in range(2):
                if not (self.is_open and self.ser and self.ser.is_open):
                    break
                self.ser.write(reply_frame)
                if self.reply_mode.get() == "hex":
                    show_reply = " ".join(f"{b:02X}" for b in reply_frame)
                else:
                    show_reply = reply_frame.decode("utf-8", errors="replace")
                self.root.after(0, self.log_tx_bytes, reply_frame, f"[自动回复{i+1}/2] {show_reply}")
                time.sleep(1.0)
        finally:
            self.reply_trigger_lock = False

    def toggle_log_display_mode(self):
        self.log_hex_mode = not self.log_hex_mode
        self.btn_log_mode.config(text="日志:HEX模式" if self.log_hex_mode else "日志:字符模式")
        self.redraw_all_log()

    def redraw_all_log(self):
        self.log.delete(1.0, tk.END)
        for item in self.log_cache:
            self._render_one_log_item(item)
        self.log.see(tk.END)

    def _render_one_log_item(self, item):
        t = item["timestamp"]
        if item["type"] == "msg":
            line = f"[{t}] {item['raw']}\n"
            self.log.insert(tk.END, line)
        elif item["type"] == "rx":
            raw_bytes = item["raw"]
            if self.log_hex_mode:
                text = " ".join(f"{b:02X}" for b in raw_bytes)
                line = f"[{t}] [RX] {text}\n"
                self.log.insert(tk.END, line)
            else:
                disp = []
                for b in raw_bytes:
                    if b in (0x0D, 0x0A):
                        disp.append(chr(b))
                    elif 32 <= b <= 126:
                        disp.append(chr(b))
                    else:
                        disp.append("·")
                disp_str = "".join(disp)
                line_head = f"[{t}] [RX] "
                self.log.insert(tk.END, line_head)
                self.log.insert(tk.END, disp_str)
                self.log.insert(tk.END, "\n")
        elif item["type"] == "tx":
            raw_bytes = item["raw"]
            show_text = item["show_text"]
            if self.log_hex_mode:
                line = f"[{t}] [TX] {show_text}\n"
                self.log.insert(tk.END, line)
            else:
                disp = []
                for b in raw_bytes:
                    if b in (0x0D, 0x0A):
                        disp.append(chr(b))
                    elif 32 <= b <= 126:
                        disp.append(chr(b))
                    else:
                        disp.append("·")
                disp_str = "".join(disp)
                line_head = f"[{t}] [TX] "
                self.log.insert(tk.END, line_head)
                self.log.insert(tk.END, disp_str)
                self.log.insert(tk.END, "\n")

    def switch_mode(self):
        lock = self.lock_cmd.get()
        state = tk.DISABLED if lock else tk.NORMAL
        if self.cmd_mode.get() == "hex":
            self.ent_prefix.config(state=state)
            self.ent_len.config(state=state)
            self.ent_payload.config(state=state)
            self.ent_str.config(state=tk.DISABLED)
        else:
            self.ent_prefix.config(state=tk.DISABLED)
            self.ent_len.config(state=tk.DISABLED)
            self.ent_payload.config(state=tk.DISABLED)
            self.ent_str.config(state=state)

    def lock_change(self):
        self.switch_mode()

    @staticmethod
    def hex2bytes(hstr):
        return bytes.fromhex(hstr.strip().replace(" ", ""))

    @staticmethod
    def xor_check(data: bytes) -> int:
        x = 0
        for b in data:
            x ^= b
        return x

    def add_cmd(self):
        if self.lock_cmd.get():
            messagebox.showwarning("提示", "已锁定，禁止修改命令！")
            return
        mode = self.cmd_mode.get()
        desc_text = self.ent_desc.get().strip()
        try:
            if mode == "hex":
                pre = self.hex2bytes(self.ent_prefix.get())
                if len(pre) != 1:
                    messagebox.showerror("错误", "前缀必须1字节HEX！")
                    return
                total_len = int(self.ent_len.get())
                payload = self.hex2bytes(self.ent_payload.get())
                len_byte = bytes([total_len])
                check_data = len_byte + payload
                xor_byte = bytes([self.xor_check(check_data)])
                full = pre + len_byte + payload + xor_byte
                show = " ".join(f"{i:02X}" for i in full)
                self.cmd_list.append({"mode": "HEX", "raw": full, "show": show, "desc": desc_text, "enable": True})
            else:
                s = self.ent_str.get()
                raw = s.encode("utf-8") + b"\r\n"
                show = s + " [+\\r\\n]"
                self.cmd_list.append({"mode": "STR", "raw": raw, "show": show, "desc": desc_text, "enable": True})
            self.refresh_tree()
            self.save_cmds()
            self.log_print(f"添加命令成功：{desc_text if desc_text else '无备注'} | {show}")
        except Exception as e:
            messagebox.showerror("解析失败", str(e))

    def del_cmd(self):
        if self.lock_cmd.get():
            messagebox.showwarning("提示", "已锁定！")
            return
        sel = self.tree.selection()
        if not sel:
            return
        idx = self.tree.index(sel[0])
        del self.cmd_list[idx]
        self.refresh_tree()
        self.save_cmds()

    def clear_cmd(self):
        if self.lock_cmd.get():
            messagebox.showwarning("提示", "已锁定！")
            return
        self.cmd_list.clear()
        self.refresh_tree()
        self.save_cmds()

    def move_up(self):
        if self.lock_cmd.get():
            messagebox.showwarning("提示", "已锁定！")
            return
        sel = self.tree.selection()
        if not sel:
            return
        idx = self.tree.index(sel[0])
        if idx <= 0:
            return
        self.cmd_list[idx], self.cmd_list[idx-1] = self.cmd_list[idx-1], self.cmd_list[idx]
        self.refresh_tree()
        items = self.tree.get_children()
        self.tree.selection_set(items[idx-1])
        self.save_cmds()

    def move_down(self):
        if self.lock_cmd.get():
            messagebox.showwarning("提示", "已锁定！")
            return
        sel = self.tree.selection()
        if not sel:
            return
        idx = self.tree.index(sel[0])
        if idx >= len(self.cmd_list)-1:
            return
        self.cmd_list[idx], self.cmd_list[idx+1] = self.cmd_list[idx+1], self.cmd_list[idx]
        self.refresh_tree()
        items = self.tree.get_children()
        self.tree.selection_set(items[idx+1])
        self.save_cmds()

    def refresh_tree(self):
        for i in self.tree.get_children():
            self.tree.delete(i)
        for d in self.cmd_list:
            tick_text = "☑" if d.get("enable", True) else "☐"
            self.tree.insert("", tk.END, values=(tick_text, d["mode"], d["desc"], d["show"]), tags=(self.tree_tag_normal,))

    def refresh_com(self):
        try:
            ports = [p.device for p in list_ports.comports()]
            self.com_cb["values"] = ports
            if ports:
                self.com_cb.current(0)
        except Exception:
            pass

    def toggle_com(self):
        if not self.is_open:
            port = self.com_cb.get()
            if not port:
                messagebox.showwarning("提示", "请选择串口")
                return
            if self.ser is not None:
                try:
                    self.ser.close()
                except Exception:
                    pass
                self.ser = None
            try:
                self.ser = serial.Serial(port, self.baudrate, self.bytesize, self.parity, self.stopbits, timeout=0.05)
                self.is_open = True
                self.btn_com.config(text="关闭串口", bg="#008800")
                self.log_print(f"✅ 串口打开成功 {port} 115200 8N1")
                self.read_running = True
                self.read_thread = threading.Thread(target=self.read_loop, daemon=True)
                self.read_thread.start()
            except Exception as e:
                messagebox.showerror("打开失败", str(e))
                return
        else:
            self.is_sending = False
            self.read_running = False
            if self.ser:
                try:
                    self.ser.close()
                except Exception:
                    pass
            self.ser = None
            self.is_open = False
            self.btn_com.config(text="打开串口", bg="#880000")
            self.log_print("❌ 串口已关闭")

    def read_loop(self):
        while self.read_running and self.ser and self.ser.is_open:
            try:
                data = self.ser.read(1024)
                if data:
                    self.root.after(0, self.log_rx_bytes, data)
                    self.save_rx_to_log(data)
                    if self.enable_auto_reply.get() and self.is_open and self.ser and self.ser.is_open:
                        if not self.reply_trigger_lock:
                            self.reply_trigger_lock = True
                            try:
                                reply_bytes = self.build_auto_reply_bytes()
                                t = threading.Thread(target=self.reply_worker_two_times, args=(reply_bytes,), daemon=True)
                                t.start()
                            except Exception as e:
                                self.reply_trigger_lock = False
                                self.root.after(0, self.log_print, f"⚠️自动回复组装失败:{str(e)}")
            except Exception:
                break
            time.sleep(0.02)

    def log_rx_bytes(self, raw_bytes: bytes):
        t = time.strftime("%H:%M:%S")
        item = {"type":"rx","raw":raw_bytes,"timestamp":t}
        self.log_cache.append(item)
        self._render_one_log_item(item)
        self.log.see(tk.END)

    def log_tx_bytes(self, raw_bytes: bytes, show_text: str):
        t = time.strftime("%H:%M:%S")
        item = {"type":"tx","raw":raw_bytes,"show_text":show_text,"timestamp":t}
        self.log_cache.append(item)
        self._render_one_log_item(item)
        self.log.see(tk.END)

    def log_print(self, msg):
        t = time.strftime("%H:%M:%S")
        item = {"type":"msg","raw":msg,"timestamp":t}
        self.log_cache.append(item)
        self._render_one_log_item(item)
        self.log.see(tk.END)

    def start_send(self):
        if not self.is_open:
            messagebox.showwarning("提示", "请先打开串口")
            return
        if not self.cmd_list:
            messagebox.showwarning("提示", "命令列表为空")
            return
        if self.is_sending:
            return
        self.is_sending = True
        interval = self.send_interval.get() / 1000.0
        def worker():
            self.log_print("========== 开始批量发送（仅发送勾选启用命令） ==========")
            for item in self.cmd_list:
                if not self.is_sending or not self.is_open:
                    break
                if not item.get("enable", True):
                    continue
                self.ser.write(item["raw"])
                self.root.after(0, self.log_tx_bytes, item["raw"], item["show"])
                time.sleep(interval)
            self.is_sending = False
            self.log_print("========== 发送完成 ==========")
        threading.Thread(target=worker, daemon=True).start()

    def stop_send(self):
        self.is_sending = False
        self.log_print("🛑 已手动停止发送")

    def clear_log(self):
        self.log.delete(1.0, tk.END)
        self.log_cache.clear()

    def save_cmds(self):
        out = []
        for d in self.cmd_list:
            out.append({
                "mode": d["mode"],
                "raw": list(d["raw"]),
                "show": d["show"],
                "desc": d["desc"],
                "enable": d.get("enable", True)
            })
        try:
            with open(self.save_file, "w", encoding="utf-8") as f:
                json.dump(out, f, ensure_ascii=False, indent=2)
        except Exception:
            pass

    def load_auto_cmds(self):
        if not os.path.exists(self.save_file):
            return
        try:
            with open(self.save_file, "r", encoding="utf-8") as f:
                data = json.load(f)
                self.cmd_list.clear()
                for d in data:
                    self.cmd_list.append({
                        "mode": d["mode"],
                        "raw": bytes(d["raw"]),
                        "show": d["show"],
                        "desc": d.get("desc", ""),
                        "enable": d.get("enable", True)
                    })
                self.refresh_tree()
                self.log_print(f"✅ 自动加载 {len(self.cmd_list)} 条保存命令")
        except Exception:
            pass

    def on_tree_double_click(self, event):
        if self.lock_cmd.get():
            messagebox.showwarning("提示", "命令已锁定，不可修改！")
            return
        item = self.tree.focus()
        if not item:
            return
        idx = self.tree.index(item)
        self.edit_cmd_popup(idx)

    def edit_cmd_popup(self, idx):
        origin = self.cmd_list[idx]
        win = tk.Toplevel(self.root)
        win.title(f"修改命令 第{idx+1}条")
        win.geometry("560x320")
        win.configure(bg=BG_COLOR)
        win.transient(self.root)
        win.grab_set()
        # 子窗口也加载图标
        try:
            if os.path.exists(self.ico_path):
                win.iconbitmap(self.ico_path)
        except Exception:
            pass

        self.root.update_idletasks()
        main_x = self.root.winfo_x()
        main_y = self.root.winfo_y()
        main_w = self.root.winfo_width()
        main_h = self.root.winfo_height()
        win_w = 560
        win_h = 320
        pos_x = main_x + (main_w - win_w) // 2
        pos_y = main_y + (main_h - win_h) // 2
        win.geometry(f"{win_w}x{win_h}+{pos_x}+{pos_y}")
        edit_mode = tk.StringVar(value=origin["mode"])
        var_desc = tk.StringVar(value=origin.get("desc", ""))
        frame_top = ttk.Frame(win)
        frame_top.pack(pady=8, padx=8, fill="x")
        ttk.Radiobutton(frame_top, text="HEX模式", variable=edit_mode, value="HEX").pack(side=tk.LEFT, padx=10)
        ttk.Radiobutton(frame_top, text="字符串模式", variable=edit_mode, value="STR").pack(side=tk.LEFT)
        ttk.Label(frame_top, text="命令含义备注：").pack(side=tk.LEFT, padx=10)
        e_desc = ttk.Entry(frame_top, textvariable=var_desc, width=28)
        e_desc.pack(side=tk.LEFT)
        frame_hex = ttk.LabelFrame(win, text="HEX参数")
        frame_str = ttk.LabelFrame(win, text="字符串参数")
        ttk.Label(frame_hex, text="前缀(1字节HEX):").grid(row=0, column=0, padx=4, pady=6, sticky="w")
        var_pre = tk.StringVar()
        e_pre = ttk.Entry(frame_hex, textvariable=var_pre)
        e_pre.grid(row=0, column=1, padx=4, pady=6, sticky="ew")
        ttk.Label(frame_hex, text="总长度(十进制):").grid(row=1, column=0, padx=4, pady=6, sticky="w")
        var_len = tk.StringVar()
        e_len = ttk.Entry(frame_hex, textvariable=var_len)
        e_len.grid(row=1, column=1, padx=4, pady=6, sticky="ew")
        ttk.Label(frame_hex, text="HEX载荷:").grid(row=2, column=0, padx=4, pady=6, sticky="w")
        var_payload = tk.StringVar()
        e_payload = ttk.Entry(frame_hex, textvariable=var_payload)
        e_payload.grid(row=2, column=1, padx=4, pady=6, sticky="ew")
        frame_hex.grid_columnconfigure(1, weight=1)
        ttk.Label(frame_str, text="字符串内容(自动追加\\r\\n):").grid(row=0, column=0, padx=4, pady=6, sticky="w")
        var_strtext = tk.StringVar()
        e_strtext = ttk.Entry(frame_str, textvariable=var_strtext)
        e_strtext.grid(row=0, column=1, padx=4, pady=6, sticky="ew")
        frame_str.grid_columnconfigure(1, weight=1)
        def update_panel():
            if edit_mode.get() == "HEX":
                frame_str.pack_forget()
                frame_hex.pack(padx=8, pady=4, fill="both", expand=True)
            else:
                frame_hex.pack_forget()
                frame_str.pack(padx=8, pady=4, fill="both", expand=True)
        edit_mode.trace_add("write", lambda *args: update_panel())
        if origin["mode"] == "HEX":
            raw = origin["raw"]
            pre_byte = raw[0:1]
            len_byte = raw[1:2]
            payload_bytes = raw[2:-1]
            var_pre.set(pre_byte.hex().upper())
            var_len.set(str(len_byte[0]))
            var_payload.set(" ".join(f"{b:02X}" for b in payload_bytes))
            edit_mode.set("HEX")
        else:
            # 直接从raw字节去掉末尾b"\r\n"解码原始文本，不再依赖show字符串截取
            raw_bytes = origin["raw"]
            if raw_bytes.endswith(b"\r\n"):
                origin_text_bytes = raw_bytes[:-2]
            else:
                origin_text_bytes = raw_bytes
            origin_text = origin_text_bytes.decode("utf-8", errors="replace")
            var_strtext.set(origin_text)
            edit_mode.set("STR")
        update_panel()
        def save_edit():
            try:
                new_item = {}
                desc_save = var_desc.get().strip()
                if edit_mode.get() == "HEX":
                    pre = self.hex2bytes(var_pre.get())
                    if len(pre) != 1:
                        messagebox.showerror("错误", "前缀必须1字节HEX！", parent=win)
                        return
                    total_len = int(var_len.get())
                    payload = self.hex2bytes(var_payload.get())
                    len_b = bytes([total_len])
                    check_data = len_b + payload
                    xor_b = bytes([self.xor_check(check_data)])
                    full_raw = pre + len_b + payload + xor_b
                    show_str = " ".join(f"{i:02X}" for i in full_raw)
                    new_item = {"mode":"HEX", "raw":full_raw, "show":show_str, "desc": desc_save, "enable": origin.get("enable", True)}
                else:
                    s_txt = var_strtext.get()
                    raw_data = s_txt.encode("utf-8") + b"\r\n"
                    show_str = s_txt + " [+\\r\\n]"
                    new_item = {"mode":"STR", "raw":raw_data, "show":show_str, "desc": desc_save, "enable": origin.get("enable", True)}
                self.cmd_list[idx] = new_item
                self.refresh_tree()
                self.save_cmds()
                self.log_print(f"✅ 修改命令成功：{desc_save if desc_save else '无备注'} | {new_item['show']}")
                win.destroy()
            except Exception as err:
                messagebox.showerror("输入解析失败", str(err), parent=win)
        frame_btn = ttk.Frame(win)
        frame_btn.pack(pady=10)
        ttk.Button(frame_btn, text="保存修改", command=save_edit).grid(row=0, column=0, padx=10)
        ttk.Button(frame_btn, text="取消", command=win.destroy).grid(row=0, column=1, padx=10)

    def on_close(self):
        self.is_sending = False
        self.read_running = False
        if self.ser:
            try:
                self.ser.close()
            except Exception:
                pass
        self.save_cmds()
        self.save_note_data()
        self.root.destroy()


if __name__ == "__main__":
    root = tk.Tk()
    app = SerialCmdTool(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()
