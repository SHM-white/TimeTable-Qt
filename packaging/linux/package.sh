#!/usr/bin/env bash
# Build Ubuntu 24.04 AMD64 packages from an already-tested CMake build.
set -euo pipefail

if [[ $# -ne 2 || ! -f CMakeLists.txt ]]; then
  echo 'Usage (from repository root): bash packaging/linux/package.sh <build-dir> <Qt5|Qt6>' >&2
  exit 2
fi

build_dir=$1
flavor=$2
case "$flavor" in
  Qt5) depends='libqt5widgets5t64, libqt5network5t64, libc6, libstdc++6' ;;
  Qt6) depends='libqt6widgets6t64, libqt6network6t64, qt6-qpa-plugins, libc6, libstdc++6' ;;
  *) echo "Unsupported Qt flavor: $flavor" >&2; exit 2 ;;
esac

version=$(sed -nE 's/^[[:space:]]*project\(TimeTableNext[[:space:]]+VERSION[[:space:]]+([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' CMakeLists.txt | head -n 1)
[[ -n "$version" ]] || { echo 'Unable to read project version' >&2; exit 1; }
[[ "$(dpkg --print-architecture)" == amd64 ]] || { echo 'Only Ubuntu x86_64 (amd64) is supported by this CI package' >&2; exit 1; }
exe="$build_dir/bin/TimeTableNext"
[[ -x "$exe" ]] || { echo "Missing executable: $exe" >&2; exit 1; }

out="TimeTableNext-v$version-Linux-$flavor"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
root="$work/deb"
install -Dm755 "$exe" "$root/usr/bin/TimeTableNext"
install -Dm644 README-NEXT.md "$root/usr/share/doc/timetable-next/README.md"
install -Dm644 LICENSE "$root/usr/share/doc/timetable-next/LICENSE"
mkdir -p "$root/DEBIAN" "$root/usr/share/applications"
cat > "$root/usr/share/applications/timetable-next.desktop" <<'DESKTOP'
[Desktop Entry]
Type=Application
Name=TimeTable Next
Comment=Floating desktop timetable and schedule manager
Exec=TimeTableNext
Terminal=false
Categories=Office;Utility;
DESKTOP
cat > "$root/DEBIAN/control" <<CONTROL
Package: timetable-next
Version: $version
Section: utils
Priority: optional
Architecture: amd64
Maintainer: TimeTable-Qt Contributors
Depends: $depends
Description: TimeTable Next desktop floating timetable ($flavor)
 Ubuntu 24.04 build with Qt shared libraries provided by the system.
CONTROL

dpkg-deb --build --root-owner-group "$root" "$out-amd64.deb"
test "$(dpkg-deb --field "$out-amd64.deb" Version)" = "$version"

mkdir -p "$work/tar/TimeTableNext"
install -m755 "$exe" "$work/tar/TimeTableNext/TimeTableNext"
install -m644 README-NEXT.md "$work/tar/TimeTableNext/README.md"
install -m644 LICENSE "$work/tar/TimeTableNext/LICENSE"
tar -czf "$out-x86_64.tar.gz" -C "$work/tar" TimeTableNext
tar -tzf "$out-x86_64.tar.gz" >/dev/null
printf 'Created %s-amd64.deb and %s-x86_64.tar.gz\n' "$out" "$out"
