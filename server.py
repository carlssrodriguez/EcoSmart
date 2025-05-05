from flask import Flask, request
from datetime import datetime

app = Flask(__name__)

# Open CSV log file and write header (only if empty)
log_file_path = "data_log.csv"
try:
    with open(log_file_path, "x") as f:
        f.write("timestamp,light_level,motion\n")
except FileExistsError:
    pass  # File already exists, no need to write header

@app.route("/")
def index():
    """Main route to receive sensor data via HTTP GET"""
    var = request.args.get("var")
    print(f"Received: {var}")

    if var:
        try:
            parts = var.split('-')
            light = parts[0].split(':')[1]
            motion = parts[1].split(':')[1]

            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            with open(log_file_path, "a") as log_file:
                log_file.write(f"{timestamp},{light},{motion}\n")
            print(f"Logged at {timestamp}: Light={light}, Motion={motion}")
        except Exception as e:
            print(f"Error parsing input: {e}")

    return "Data received"

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
