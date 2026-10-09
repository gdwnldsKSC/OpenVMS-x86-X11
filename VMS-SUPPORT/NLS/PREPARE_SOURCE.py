"""Package the exact modular libX11 NLS and runtime-data build inputs."""

import argparse
import hashlib
import io
import json
from pathlib import Path
import re
import zipfile


def inputs(root):
    files = [
        "lib/libX11/nls/Makefile.am",
        "lib/libX11/nls/localerules.in",
        "lib/libX11/cpprules.in",
        "lib/libX11/nls/locale.alias.pre",
        "lib/libX11/nls/locale.dir.pre",
        "lib/libX11/nls/compose.dir.pre",
        "lib/libX11/src/XErrorDB",
        "lib/libX11/src/XKeysymDB",
        "lib/libX11/src/xcms/Xcms.txt",
        "VMS-SUPPORT/CONFIG.MMS",
        "VMS-SUPPORT/COMPILE.COM",
        "VMS-SUPPORT/LIB/X11/DATA.MMS",
        "VMS-SUPPORT/NLS/DESCRIP.MMS",
        "VMS-SUPPORT/NLS/GENERATE.COM",
        "VMS-SUPPORT/NLS/GENERATE_SET.COM",
        "VMS-SUPPORT/NLS/LINK_FILTER.COM",
        "VMS-SUPPORT/NLS/filter.c",
        "VMS-SUPPORT/NLS/XLC_LOCALE.LIST",
        "VMS-SUPPORT/NLS/COMPOSE.LIST",
    ]
    for kind, filename in (("XLC_LOCALE", "XLC_LOCALE.pre"), ("COMPOSE", "Compose.pre")):
        listing = root / "VMS-SUPPORT" / "NLS" / (kind + ".LIST")
        for name in listing.read_text(encoding="ascii").splitlines():
            if not re.fullmatch(r"[A-Za-z0-9_.-]+", name) or name in (".", ".."):
                raise ValueError("Invalid locale source name: " + repr(name))
            files.append("lib/libX11/nls/" + name + "/" + filename)
    if len(files) != len(set(files)):
        raise ValueError("Duplicate source input")
    return sorted(files)


def package(root):
    payload = io.BytesIO()
    manifest = []
    destinations = set()
    with zipfile.ZipFile(payload, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for relative in inputs(root):
            source = root / relative
            if not source.resolve().is_relative_to(root):
                raise ValueError("Source escapes tree: " + relative)
            data = source.read_bytes()
            destination = relative
            if destination.casefold() in destinations:
                raise ValueError("Duplicate native input: " + destination)
            destinations.add(destination.casefold())
            info = zipfile.ZipInfo(destination, date_time=(1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.create_system = 3
            info.external_attr = 0o100644 << 16
            archive.writestr(info, data)
            manifest.append({
                "source": relative,
                "destination": destination,
                "bytes": len(data),
                "sha256": hashlib.sha256(data).hexdigest(),
            })
    packed = payload.getvalue()
    return packed, {"zip_sha256": hashlib.sha256(packed).hexdigest(), "inputs": manifest}


def main():
    parser = argparse.ArgumentParser(
        description="Create an exact-byte NLS/Xlib-data source ZIP for a native build tree. "
                    "Extract into the native source root, then use "
                    "the NLS and X11_DATA build targets. No runtime data or executable is installed.")
    parser.add_argument("output", type=Path, help="New ZIP file; existing files are never replaced")
    parser.add_argument("--source-root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--manifest", type=Path, help="Optional new JSON input-hash manifest")
    args = parser.parse_args()
    root = args.source_root.resolve(strict=True)
    outputs = [args.output] + ([args.manifest] if args.manifest else [])
    resolved = [path.resolve() for path in outputs]
    if len(set(resolved)) != len(resolved):
        parser.error("ZIP and manifest must have different paths")
    for path in outputs:
        if path.exists():
            parser.error("Output already exists: " + str(path))
        if not path.parent.is_dir():
            parser.error("Output directory does not exist: " + str(path.parent))
    packed, manifest = package(root)
    with args.output.open("xb") as output:
        output.write(packed)
    if args.manifest:
        with args.manifest.open("x", encoding="utf-8", newline="\n") as output:
            json.dump(manifest, output, indent=2)
            output.write("\n")
    print("Packaged", len(manifest["inputs"]), "inputs; SHA256", manifest["zip_sha256"])


if __name__ == "__main__":
    main()
