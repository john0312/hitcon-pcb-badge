import subprocess
import os


def get_gnome_scaling_factor():
    try:
        result = subprocess.run(
            ['gsettings', 'get', 'org.gnome.desktop.interface', 'scale-factor'],
            capture_output=True, text=True
        )
        scaling_factor = result.stdout.strip()
        return scaling_factor
    except Exception as e:
        print(f"Error getting GNOME scaling factor: {e}")
        return None

def get_kde_scaling_factor():
    try:
        kdeglobals_path = os.path.expanduser('~/.config/kdeglobals')
        if not os.path.isfile(kdeglobals_path):
            print("KDE configuration file not found.")
            return None
        
        with open(kdeglobals_path, 'r') as file:
            for line in file:
                if 'ScaleFactor' in line:
                    return line.split('=')[1].strip()
        return '1.0'
    except Exception as e:
        print(f"Error reading KDE scaling factor from kdeglobals: {e}")
        return None

def get_wayland_scaling_factor():
    try:
        result = subprocess.run(
            ['swaymsg', '-t', 'get', 'outputs'],
            capture_output=True, text=True
        )
        # Look for scaling in the output
        for line in result.stdout.split('\n'):
            if 'scale' in line:
                return line.split(':')[1].strip()
        return None
    except Exception as e:
        print(f"Error getting Wayland scaling factor: {e}")
        return None

def get_env_variable(var_name):
    return os.environ.get(var_name, None)

def detect_desktop_environment():
    desktop_session = get_env_variable('DESKTOP_SESSION')
    xdg_current_desktop = get_env_variable('XDG_CURRENT_DESKTOP')

    if desktop_session:
        print(f"DESKTOP_SESSION: {desktop_session}")
    if xdg_current_desktop:
        print(f"XDG_CURRENT_DESKTOP: {xdg_current_desktop}")

    if 'GNOME' in xdg_current_desktop:
        return 'GNOME'
    elif 'KDE' in xdg_current_desktop or 'KDE Plasma' in xdg_current_desktop:
        return 'KDE'
    elif 'Sway' in xdg_current_desktop or 'Wayfire' in xdg_current_desktop:
        return 'Wayland'
    else:
        print(f"Detected desktop environment: {xdg_current_desktop}")
        return None

def get_scaling_factor():
    desktop_env = detect_desktop_environment()
    if desktop_env == 'GNOME':
        return get_gnome_scaling_factor()
    elif desktop_env == 'KDE':
        return get_kde_scaling_factor()
    elif desktop_env == 'Wayland':
        return get_wayland_scaling_factor()
    else:
        print("Unsupported desktop environment or failed to detect.")
        return 1.0


if __name__ == '__main__':
    scaling_factor = get_scaling_factor()
    print(f'Scaling factor: {scaling_factor}')
