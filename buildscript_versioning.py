import datetime
import re
import os

# File path for the version header
FILENAME_VERSION_H = 'include/version.h'

# Define the base version prefix (to ensure consistency if the file doesn't exist)
BASE_VERSION_PREFIX = "3.0"

# Read the current version
current_version = f"{BASE_VERSION_PREFIX}.0"  # Default version if file not found

if os.path.exists(FILENAME_VERSION_H):
    with open(FILENAME_VERSION_H, 'r') as f:
        content = f.read()
        match = re.search(r'#define VERSION "(.*)"', content)
        if match:
            current_version = match.group(1)
        else:
            print(f"VERSION not found in {FILENAME_VERSION_H}. Starting with base version.")

print(f"Current version: {current_version}")

# Increment the version
parts = current_version.split('.')
if len(parts) < 3:
    raise ValueError("Version format is invalid. Expected format 'X.Y.Z'")
parts[-1] = str(int(parts[-1]) + 1)  # Increment the last segment
new_version = '.'.join(parts)
print(f"New version: {new_version}")

# Generate the current build date
build_date = datetime.datetime.now().strftime('%Y-%m-%d %H:%M')

# Prepare the header content
header_content = f"""
#ifndef VERSION
  #define VERSION "{new_version}"
#endif
#ifndef BUILD_DATE
  #define BUILD_DATE "{build_date}"
#endif
"""

# Ensure the directory exists
os.makedirs(os.path.dirname(FILENAME_VERSION_H), exist_ok=True)

# Write the updated version and build date to the header file
with open(FILENAME_VERSION_H, 'w') as f:
    f.write(header_content)
    print(f"Updated {FILENAME_VERSION_H} with version {new_version} and build date {build_date}")
