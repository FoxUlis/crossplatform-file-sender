import subprocess
import threading
import time
import sys
import os
from rich.console import Console
from rich.layout import Layout
from rich.panel import Panel
from rich.live import Live
from rich.prompt import Prompt, IntPrompt
from rich.text import Text
from rich.progress import Progress, SpinnerColumn, TextColumn, BarColumn, TaskProgressColumn

console = Console()

# Binary path
BINARY_PATH = "./file_transfer"

class TransferProcess:
    def __init__(self, mode, filename=None, save_as=None, ip="127.0.0.1", port="4444"):
        self.mode = mode #'server' or 'client'
        self.filename = filename
        self.save_as = save_as
        self.ip = ip
        self.port = port
        self.process = None
        self.output_lines = []
        self.is_running = False
        self.status = "Initializing..."
        self.progress_text = ""
    
    def start(self):
        self.is_running = True
        self.process = subprocess.Popen(
                [BINARY_PATH],
                stdin = subprocess.PIPE,
                stdout = subprocess.PIPE,
                stderr = subprocess.STDOUT,
                text = True,
                bufsize = 1
        )

        #Thread for reading output
        self.reader_thread = threading.Thread(target = self._read_output)
        self.reader_thread.daemon = True
        self.reader_thread.start()

        #Thread fo sending input (to avoid blocking the UI)
        self.writer_thread = threading.Thread(targer = self._send_input)
        self.writer_thread.daemon = True
        self.writer_thread.start()

    def _send_input(self):
        try:
            if self.mode == "server":
                self.process.stdin.write("2\n") #Server mode
                self.process.stdin.flush()
                time.sleep(1)
                if self.filename:
                    self.process.stdin.write(f"{self.filename}\n")
                    self.process.stdin.flush()
            else:
                self.process.stdin.write("1\n") #Client mode
                self.process.stdin.flush()
                time.sleep(1)
                if self.save_as:
                    self.process.stdin.write(f"{save.save_as}\n")
                    self.process.stdin.flush()
                    time.sleep(1)
                if self.ip:
                    self.process.stdin.write(f"{self.ip}\n")
                    self.process.stdin.flush()
                    time.sleep(1)
                if self.port:
                    self.process.stdin.write(f"{port}\n")
                    self.process.stdin.flush()

        except Exception as e:
            self.output_lines.append(f"[red]Input Error: {e}[/red]")

    def _read_output(self):
        for line in self.process.stdout:
            line = line.strip()
            if not line: continue

            #Save log
            self.output_lines.append(line)
            if len(self.output_lines) > 20: #Save last 20 strings
                self.output_lines.pop(0)

            #Parsing status for UI
            if "Waiting for incoming connections" in line:
                self.status = "[yellow]Waiting for client...[/yellow]"
            elif "Client has connected" in line:
                self.status = "[green]Client connected! Starting transfer...[/green]"
            elif "Sending filesize" in line or "Receiving filesize" in line:
                self.status = "[blue]Negotiating metadata...[/blue]"
            elif "Sent:" in line:
                self.status = "[cyan]Sending Data...[/cyan]"
                self.progress_text = line.replace("Sent:", "").strip()
            elif "Receiving" in line:
                self.status = "[cyan]Receiving Data...[/cyan]"
                self.progress_text = line.replace("Receiving:", "").strip()
            elif "successfully sent" in line or "successfully received" in line:
                self.status = "[bold green]TRANSFER COMPLETE![/bold green]"
                self.is_running = False
            elif "Error" in line or "failed" in line.lower():
                self.status = f"[bold red]ERROR: {line}[/bold red]"
                self.is_running = False

    def stop(self):
        if self.process and self.process.poll() in None:
            self.process.terminate()
        self.is_running = False

def get_file_list():
    """Получает список файлов в текущей директории"""
    files = [f for f in os.listdir('.') if os.path.isfile(f)]
    return sorted(files)

def main():
    console.print("[bold blue]Cross-platforn File Sender TUI[/bold blue]")

    #Role choices
    role = Prompt.ask("Select Role", choices = ["server", "client"], default = "server")

    transfer = None

    if role == "server":
        #File choices for send
        files = get_file_list()
        console.print("\n[bold]Availeble files:[/bold]")
        for i, f in enumerate(files):
            size = os.path.getsize(f) / (1024*1024)
            console.print(f" {i+1}. {f} ({size:.2f} MB)")

        choice = IntPrompt.ask("Select file number", default = 1)
        if 1 <= choice <= len(files):
            selected_file = files[choice - 1]
        else:
            selected_file = Prompt.ask("Or enter filename manually")

        console.print(f"\n[green]Starting Server with file: {selected_file}[/green]")
        transfer = TransferProcess(mode = "server", filename = selected_file)

    else:
        #Client settings
        save_name = Prompt.ask("Save file as", default="received_file")
        ip = Prompt.ask("Server IP", default = "127.0.0.1")
        port = Prompt.ask("Port", default = "4444")

        console.print(f"\n[blue]Connecting to {ip}:{port}...[/blue]")
        transfer = TransferProcess(mode = "client", save_as = save_name, ip = ip, port = port)

    #Process start
    try:
        with Live(console = console, refresh_per_second = 10) as live:
            while transfer.is_running:
                layout = Layout()

                #Top pannel: Status
                status_panel = Panel(
                        Text(transfer.status, justify = "center"),
                        title = "[bold]Status[/bold]",
                        border_style = "blue"
                )

                #Middle pannel: Progress
                progress_content = transfer.progress_text if transfer.progress_text else "Waiting for data..."
                progress_panel = Panel(
                        Text(progress_content, justify = "center", style = "cyan"),
                        title = "[bold]Transfer progress[/bold]",
                        border_style = "cyan"
                )

                #Bottom pannel: Logs
                logs = "\n".join(transfer.output_lines[-8:]) # Last 8 strings
                log_panel = Panel(
                        Text(logs, style = "dim white"),
                        title = "[bold]Logs[/bold]",
                        border_style = "white"
                )

                layout.split_column(
                        Layout(status_panel, size = 3),
                        Layout(progress_panel, size = 5),
                        Layout(log_panel)
                )

                live.update(layout)
                time.sleep(0.1)

            # Final update after finish
            layout = Layout()
            status_panel = Panel(Text(transfer.status, justify = "centet"), title = "Status", border_style = "green")
            layout.split_column(Layout(status_panel))
            live.update(layout)
            time.sleep(2)

    except KeyboardInterrupt:
        console.print("\n[yellow]Interrupted by user.[/yellow]")
    finally:
        transfer.stop()

if __name__ == "__main__":
    main()



