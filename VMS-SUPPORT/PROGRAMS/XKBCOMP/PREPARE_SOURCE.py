"""Package the exact upstream-installed XKB data for a native source tree."""

import argparse
import hashlib
import io
import json
from pathlib import Path
import re
import zipfile


def input_mappings(root):
    """Follow this release's native (not cross-compile) Imake data install rules."""
    base = root / "programs" / "xkbcomp"
    mappings = []
    imakefiles = []
    visited = set()

    def visit(relative):
        if relative in visited:
            raise ValueError("Repeated XKB data directory: " + relative)
        visited.add(relative)
        directory = base / relative
        filename = directory / "Imakefile"
        imakefiles.append(filename.relative_to(root).as_posix())
        text = filename.read_text(encoding="ascii").replace("\\\n", " ")
        values = {"CROSS": "", "TESTDATA": ""}
        for name in ("DATAFILES", "LISTFILES", "SUBDIRS"):
            found = re.findall(r"^\s*" + name + r"\s*=([^\n]*)", text, re.M)
            if len(found) > 1:
                raise ValueError("Ambiguous Imake assignment: " + str(filename))
            value = found[0].strip() if found else ""
            value = re.sub(r"\$\((CROSS|TESTDATA)\)", lambda m: values[m[1]], value)
            words = value.split()
            if any(not re.fullmatch(r"[A-Za-z0-9_.-]+", word) or word in (".", "..") for word in words):
                raise ValueError("Unsupported Imake input: " + value)
            values[name] = words
        for kind in ("DATAFILES", "LISTFILES"):
            if values[kind] and "InstallMultiple($(" + kind + ")," not in text:
                raise ValueError("Missing data install rule: " + str(filename))
            for leaf in values[kind]:
                path = (Path(relative) / leaf).as_posix()
                mappings.append((path, path))
        links = re.findall(r"InstallCreateLink\(\$\(LIBDIR\)/xkb/([^,]+),([^,]+),([^\)]+)\)", text)
        for target_directory, source, destination in links:
            if target_directory != relative:
                raise ValueError("Unexpected link directory")
            for leaf in (source, destination):
                if not re.fullmatch(r"[A-Za-z0-9_.-]+", leaf) or leaf in (".", ".."):
                    raise ValueError("Unsupported link name")
            mappings.append((relative + "/" + source, relative + "/" + destination))
        for child in values["SUBDIRS"]:
            visit((Path(relative) / child).as_posix())

    visit("")
    mappings.sort(key=lambda pair: pair[1].casefold())
    destinations = [destination.casefold() for _, destination in mappings]
    if len(destinations) != len(set(destinations)):
        raise ValueError("Case-insensitive XKB data collision")
    for source, _ in mappings:
        filename = base / source
        if not filename.resolve(strict=True).is_relative_to(base.resolve()):
            raise ValueError("Source escapes XKB tree: " + source)
    return mappings, sorted(imakefiles)


def listing(mappings):
    return "! Upstream xkbcomp Imakefile installation: source|destination\n" + "".join(
        source + "|" + destination + "\n" for source, destination in mappings)


def package(root):
    mappings, imakefiles = input_mappings(root)
    controls = ["DATA.MMS", "DATA.LIST", "STAGE_DATA.COM", "PREPARE_SOURCE.py"]
    support = "VMS-SUPPORT/PROGRAMS/XKBCOMP/"
    if (root / support / "DATA.LIST").read_text(encoding="ascii") != listing(mappings):
        raise ValueError("DATA.LIST does not match the upstream Imakefiles")
    files = sorted(set(imakefiles + ["programs/xkbcomp/" + source for source, _ in mappings]
                       + [support + name for name in controls]))
    payload = io.BytesIO()
    manifest = []
    with zipfile.ZipFile(payload, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for relative in files:
            filename = root / relative
            if not filename.resolve(strict=True).is_relative_to(root):
                raise ValueError("Source escapes tree: " + relative)
            data = filename.read_bytes()
            info = zipfile.ZipInfo(relative, date_time=(1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.create_system = 3
            info.external_attr = 0o100644 << 16
            archive.writestr(info, data)
            manifest.append({"source": relative, "bytes": len(data),
                             "sha256": hashlib.sha256(data).hexdigest()})
    packed = payload.getvalue()
    return packed, {"zip_sha256": hashlib.sha256(packed).hexdigest(),
                    "data_files": len(mappings), "inputs": manifest}


def main():
    parser = argparse.ArgumentParser(description=
        "Package upstream-installed XKB data and native staging controls. Extract into "
        "the native source root, then build XKB_DATA. It uses the upstream install layout "
        "with byte-identical copies for the four xfree86 rule aliases; no upstream source "
        "is modified and no generated component .dir file collides with its directory.")
    parser.add_argument("output", type=Path, help="New ZIP output; never overwrite")
    parser.add_argument("--source-root", type=Path, default=Path(__file__).resolve().parents[3])
    parser.add_argument("--manifest", type=Path, help="Optional new input-hash manifest")
    args = parser.parse_args()
    root = args.source_root.resolve(strict=True)
    outputs = [args.output] + ([args.manifest] if args.manifest else [])
    if len(set(path.resolve() for path in outputs)) != len(outputs):
        parser.error("ZIP and manifest must be different files")
    for path in outputs:
        if path.exists() or not path.parent.is_dir():
            parser.error("Output must be new and its directory must exist: " + str(path))
    packed, manifest = package(root)
    with args.output.open("xb") as output:
        output.write(packed)
    if args.manifest:
        with args.manifest.open("x", encoding="utf-8", newline="\n") as output:
            json.dump(manifest, output, indent=2)
            output.write("\n")
    print("Packaged", manifest["data_files"], "XKB data files; SHA256", manifest["zip_sha256"])


if __name__ == "__main__":
    main()
