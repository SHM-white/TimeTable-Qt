#!/usr/bin/env bash
# This package must be built and tested on its target Linux distribution.
set -euo pipefail
if [[ $# -ne 2 || ! -f CMakeLists.txt ]]; then
  echo 'Usage: bash packaging/linux/package.sh <build-dir> <Qt5|Qt6>' >&2
  exit 2
fi
build_dir=$1
flavor=$2
. /etc/os-release
case "$ID:$VERSION_ID" in
  ubuntu:24.04) distribution=Ubuntu24.04 ;;
  debian:13) distribution=Debian13 ;;
  *) echo "Unsupported build distribution: $ID $VERSION_ID" >&2; exit 2 ;;
esac
case "$distribution:$flavor" in
  Ubuntu24.04:Qt5|Debian13:Qt5)
    depends='libqt5widgets5t64, libqt5network5t64, libc6, libstdc++6' ;;
  Ubuntu24.04:Qt6)
    depends='libqt6widgets6t64, libqt6network6t64, qt6-qpa-plugins, libc6, libstdc++6' ;;
  Debian13:Qt6)
    depends='libqt6widgets6, libqt6network6, qt6-qpa-plugins, libc6, libstdc++6' ;;
  *) echo "Unsupported Qt flavor: $flavor" >&2; exit 2 ;;
esac
IFS=',' read -ra dependency_list <<< "$depends"
for dependency in "${dependency_list[@]}"; do
  dependency="${dependency// /}"
  apt-cache show "$dependency" > /dev/null 2>&1 || {
    echo "Package not found in $distribution APT sources: $dependency" >&2
    exit 1
  }
done
version=$(sed -nE 's/^[[:space:]]*project\(TimeTableNext[[:space:]]+VERSION[[:space:]]+([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' CMakeLists.txt | head -n 1)
[[ -n "$version" ]] || { echo 'Unable to read project version' >&2; exit 1; }
[[ "$(dpkg --print-architecture)" == amd64 ]] || { echo 'Only AMD64 is supported' >&2; exit 1; }
exe="$build_dir/bin/TimeTableNext"
[[ -x "$exe" ]] || { echo "Executable not found: $exe" >&2; exit 1; }
out="TimeTableNext-v$version-Linux-$distribution-$flavor"
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
 Built for $distribution with system-provided Qt shared libraries.
CONTROL
dpkg-deb --build --root-owner-group "$root" "$out-amd64.deb"
test "$(dpkg-deb --field "$out-amd64.deb" Version)" = "$version"
apt-get install --simulate "./$out-amd64.deb"
mkdir -p "$work/tar/TimeTableNext"
install -m755 "$exe" "$work/tar/TimeTableNext/TimeTableNext"
install -m644 README-NEXT.md "$work/tar/TimeTableNext/README.md"
install -m644 LICENSE "$work/tar/TimeTableNext/LICENSE"
tar -czf "$out-x86_64.tar.gz" -C "$work/tar" TimeTableNext
tar -tzf "$out-x86_64.tar.gz" >/dev/null
printf 'Created %s-amd64.deb and %s-x86_64.tar.gz\n' "$out" "$out"
