#!/usr/bin/env python3
"""
OptiToken.py - Fetch OptiFine token AND download the JAR
Usage:  python OptiToken.py <jar_filename> <dest_path>
Stdout: PROGRESS:<0-100>
        DONE:<size_mb>
Stderr: error messages
"""
import sys, re, os, requests

if len(sys.argv) < 3:
    print("ERROR: Usage: OptiToken.py <jar_filename> <dest_path>", file=sys.stderr)
    sys.exit(1)

filename  = sys.argv[1]
dest_path = sys.argv[2]
ad_url    = "https://optifine.net/adloadx?f=" + filename
ua        = "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"

try:
    # Step 1: Fetch adloadx page
    res = requests.get(ad_url, headers={"User-Agent": ua}, timeout=30)
    res.raise_for_status()

    # Step 2: Parse real download URL
    # HTML uses single quotes: href='downloadx?f=...&x=TOKEN'
    # Stop at single quote, double quote, whitespace, or <
    match = re.search(r"downloadx\?[^'\"<\s]+", res.text)
    if not match:
        print("ERROR: No downloadx token found in page", file=sys.stderr)
        sys.exit(1)

    real_url = "https://optifine.net/" + match.group(0)

    # Step 3: Download JAR (keep same session for cookies)
    dl_headers = {"User-Agent": ua, "Referer": ad_url}
    r = requests.get(real_url, headers=dl_headers,
                     allow_redirects=True, stream=True, timeout=120)
    r.raise_for_status()

    total      = int(r.headers.get("content-length", 0))
    downloaded = 0

    os.makedirs(os.path.dirname(os.path.abspath(dest_path)), exist_ok=True)
    with open(dest_path, "wb") as f:
        for chunk in r.iter_content(chunk_size=65536):
            if chunk:
                f.write(chunk)
                downloaded += len(chunk)
                if total > 0:
                    pct = downloaded * 100 // total
                    print("PROGRESS:{}".format(pct), flush=True)

    size_mb = os.path.getsize(dest_path) / (1024 * 1024)
    print("DONE:{:.2f}".format(size_mb), flush=True)
    sys.exit(0)

except Exception as e:
    print("ERROR:{}".format(e), file=sys.stderr)
    sys.exit(1)
