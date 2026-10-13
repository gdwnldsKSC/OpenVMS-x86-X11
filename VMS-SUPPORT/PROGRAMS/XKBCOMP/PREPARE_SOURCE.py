"""Package the exact upstream-installed XKB data for a native source tree."""

import argparse
import hashlib
import io
import json
from pathlib import Path
import re
import zipfile


PROVIDER = "data/xkeyboard-config"
GENERATED = {"rules/" + name for name in
             ("base", "evdev", "base.lst", "evdev.lst", "base.xml", "evdev.xml",
              "base.extras.xml", "evdev.extras.xml")}


def input_mappings(root):
    """Follow xkeyboard-config's distributed component and rules installation."""
    base = root / PROVIDER
    mappings = []
    makefiles = [PROVIDER + "/" + name for name in
                 ("Makefile.am", "configure.in", "xkbrules.am", "rules/Makefile.am")]
    visited = set()

    def words(value, filename):
        result = value.split()
        if any(not re.fullmatch(r"[A-Za-z0-9_.-]+", word) or word in (".", "..")
               for word in result):
            raise ValueError("Unsupported Automake input: " + str(filename))
        return result

    def assignments(text, name, filename):
        values = re.findall(r"^" + re.escape(name) + r"\s*=([^\n]*)", text, re.M)
        if len(values) != 1:
            raise ValueError("Missing or ambiguous " + name + ": " + str(filename))
        return values[0]

    def visit(relative):
        if relative in visited:
            raise ValueError("Repeated XKB data directory: " + relative)
        visited.add(relative)
        filename = base / relative / "Makefile.am"
        makefiles.append(filename.relative_to(root).as_posix())
        text = filename.read_text(encoding="ascii").replace("\\\n", " ")
        for match in re.finditer(r"^dist_[A-Za-z0-9_]+_DATA\s*=([^\n]*)", text, re.M):
            for leaf in words(match[1], filename):
                path = (Path(relative) / leaf).as_posix()
                mappings.append((path, path))
        subdirs = re.findall(r"^SUBDIRS\s*=([^\n]*)", text, re.M)
        if len(subdirs) > 1:
            raise ValueError("Ambiguous SUBDIRS: " + str(filename))
        for child in words(subdirs[0] if subdirs else "", filename):
            visit((Path(relative) / child).as_posix())

    top = base / "Makefile.am"
    top_text = top.read_text(encoding="ascii").replace("\\\n", " ")
    subdirs = words(assignments(top_text, "SUBDIRS", top), top)
    components = ("compat", "geometry", "keycodes", "symbols", "types")
    if any(component not in subdirs for component in components) or "rules" not in subdirs:
        raise ValueError("Missing upstream XKB installation section")
    for component in components:
        visit(component)

    rules = base / "rules" / "Makefile.am"
    rules_text = rules.read_text(encoding="ascii").replace("\\\n", " ")
    variables = {}
    for name in ("rules_files", "lst_files", "dist_files", "rules_DATA"):
        value = assignments(rules_text, name, rules)
        def expand(match):
            if match[1] not in variables:
                raise ValueError("Unsupported rules variable: " + match[1])
            return variables[match[1]]
        variables[name] = re.sub(r"\$\(([A-Za-z0-9_]+)\)", expand, value)
    for leaf in words(variables["rules_DATA"], rules):
        mappings.append(("rules/" + leaf, "rules/" + leaf))
    for leaf in words(assignments(rules_text, "xml_in_files", rules), rules):
        if not leaf.endswith(".xml.in"):
            raise ValueError("Unsupported XML input: " + leaf)
        mappings.append(("rules/" + leaf[:-3], "rules/" + leaf[:-3]))

    # Upstream --with-xkb-rules-symlink=xorg installs these aliases. Native
    # staging uses byte-identical copies.
    for source, suffix, variable in (("base", "", "extra_rules_file"),
                                     ("base.lst", ".lst", "extra_lst_file"),
                                     ("base.xml", ".xml", "extra_dir_file")):
        pattern = (r"ln\s+-sf\s+" + re.escape(source) +
                   r"\s+\$\(DESTDIR\)\$\(rulesdir\)/\$\$" + variable + r"\s*;")
        if not re.search(pattern, rules_text):
            raise ValueError("Upstream rules alias hook changed")
        if ("rules/" + source, "rules/" + source) not in mappings:
            raise ValueError("Alias source is not distributed: " + source)
        mappings.append(("rules/" + source, "rules/xorg" + suffix))

    mappings.sort(key=lambda pair: pair[1].casefold())
    destinations = [destination.casefold() for _, destination in mappings]
    if len(destinations) != len(set(destinations)):
        raise ValueError("Case-insensitive XKB data collision")
    for source, _ in mappings:
        if source in GENERATED:
            continue
        filename = base / source
        if not filename.resolve(strict=True).is_relative_to(base.resolve()):
            raise ValueError("Source escapes XKB tree: " + source)
    return mappings, sorted(makefiles)


def listing(mappings):
    return "! Upstream xkeyboard-config installed data and xorg aliases: source|destination\n" + "".join(
        source + "|" + destination + "\n" for source, destination in mappings)


def package(root):
    mappings, makefiles = input_mappings(root)
    controls = ["DATA.MMS", "DATA.LIST", "STAGE_DATA.COM", "GENERATE_RULES.PL", "PREPARE_SOURCE.py"]
    support = "VMS-SUPPORT/PROGRAMS/XKBCOMP/"
    if (root / support / "DATA.LIST").read_text(encoding="ascii") != listing(mappings):
        raise ValueError("DATA.LIST does not match the upstream Makefile.am files")
    rule_inputs = [path.relative_to(root).as_posix()
                   for path in (root / PROVIDER / "rules").rglob("*") if path.is_file()]
    files = sorted(set(makefiles + [PROVIDER + "/" + source for source, _ in mappings
                                   if source not in GENERATED]
                       + rule_inputs + [support + name for name in controls]))
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
        "for its distributed data; no upstream source "
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
