import re
import json
import os
from datetime import datetime
from datetime import date
import sys

version_h = os.getcwd() + '/include/version.h'
version_json = os.getcwd() + '/include/version.json'
buildnumber_txt = os.getcwd() + '/include/buildnumber.txt'

version_string = ""
# Get current version string from the header file
with open(version_h, 'r') as file:
    for line in file:
        if "VERSION" in line:
            match = re.match(r'^(.*[^ ]) +([^ ]+) *$', line)
            if match:
                version_string = match.group(2)
                version_string = version_string.replace('"', '')

if sys.argv[1] == "getversion":
    # print current full version directly
    print(version_string)
else:
    print(f"get current version: {version_string} from {version_h}")
    # get the current buildnumber if file exists
    try:
        # Attempt to open and read the file
        with open(buildnumber_txt, 'r') as file:
            for line in file:
                build_string = line
                build_string = build_string.replace("\n","")
                print(f"got buildnumber to use: ->{build_string}<-")
    except FileNotFoundError:
        print('ERROR: buildnumber file not found. For automatic versioning there is a file called ../include/buildnumber.txt expected. With the content "localDev" or versionnumber e.g. "1.0.0" in first line. (File is blocked by .gitignore for GitHub actions to run.)')
    if "_" in version_string:
        version_string = version_string.split('_')[0]
    major, minor, patch = map(int, version_string.split('.'))

    if build_string != "localDev":        
        # using thze build number github action
        version_string_new = f"{major}.{minor}.{build_string}"
    else:
        # Increment to the new version string
        # patch += 1
        version_string_new = f"{major}.{minor}.{patch}_{build_string}"
    
    print(f"set new version: {version_string_new}")
    
    # datetime_now = datetime.now().strftime("%b %d %Y - %H:%M:%S
    datetime_now = datetime.now().strftime("%d.%m.%Y - %H:%M:%S")
    
    # Update the header file
    with open(version_h, 'w', encoding='ascii') as file:
        file.write(f'#define VERSION "{version_string_new}"\n')
        file.write(f'#define BUILDTIME "{str(datetime_now)}"\n')
        file.write(f'#define BUILDTIMESTAMP "{int((datetime.now()).timestamp())}"')
    
    # Update the JSON file
    with open(version_json, 'r') as file:
        json_data = json.load(file)
    
    json_data['version'] = version_string_new
    json_data['versiondate'] = datetime_now
    json_data['linksnapshot'] = "https://github.com/ohAnd/dtuGateway/releases/download/snapshot/energyCostSaver_ESP8266_snapshot_" + version_string_new + ".bin"
    json_data['link'] = "https://github.com/ohAnd/dtuGateway//releases/latest/download/energyCostSaver_ESP8266_release_" + version_string_new + ".bin"
    
    # Save the updated JSON data back to the file
    with open(version_json, 'w', encoding='ascii') as file:
        json.dump(json_data, file, ensure_ascii=False, indent=4)
