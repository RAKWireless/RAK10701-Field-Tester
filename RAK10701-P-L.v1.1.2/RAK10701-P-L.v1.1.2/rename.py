import sys
import shutil
import os
import json
import zipfile

try:
	with open('./.vscode/arduino.json') as f:
		data = json.load(f)
	version = data['version']
except:
	version = '0.0.0'

# print("Firmware Version " + version)

try:
	with open('./.vscode/arduino.json') as f:
		data = json.load(f)
	project = data['project']
except:
	project = data['sketch']

try:
	with open('./.vscode/arduino.json') as f:
		data = json.load(f)
	sketch = data['sketch']
except:
	sketch = 'firmware'

try:
	with open('./.vscode/arduino.json') as f:
		data = json.load(f)
	output = data['output']
except:
	output = 'build'

# Check if the destination directory exists
if not os.path.exists('./'+output): 
    os.makedirs('./'+output) 


# Specify the source file, the destination for the copy, and the new name
source_file = './'+output+'/'+sketch+'.hex'
source_file_2 = './'+output+'/'+sketch+'.zip'
destination_directory = '../Firmware/'
new_file_name = project+'_V'+version+'.hex'
new_zip_name = project+'_V'+version+'.zip'

# print("Source file HEX " + source_file)
# print("Source file ZIP " + source_file_2)
# print("Destination " + destination_directory)
# print("New HEX name " + new_file_name)
# print("New ZIP name " + new_zip_name) 

if os.path.isfile(destination_directory+new_file_name):
	try:
		os.remove(destination_directory+new_file_name)
	except:
		print('Cannot delete '+destination_directory+new_file_name)
	# finally:
	# 	print('Delete '+destination_directory+new_file_name)

if os.path.isfile(destination_directory+new_zip_name):
	try:
		os.remove(destination_directory+new_zip_name)
	except:
		print('Cannot delete '+destination_directory+new_zip_name)
	# finally:
	# 	print('Delete '+destination_directory+new_zip_name)
		
if os.path.isfile('./'+output+'/'+new_zip_name):
	try:
		os.remove('./'+output+'/'+new_zip_name)
	except:
		print('Cannot delete '+'./'+output+'/'+new_zip_name)
	# finally:
	# 	print('Delete '+'./'+output+'/'+new_zip_name)

if os.path.isfile('../Firmware/'+new_zip_name):
	try:
		os.remove('../Firmware/'+new_zip_name)
	except:
		print('Cannot delete '+'../Firmware/'+new_zip_name)
	# finally:
	# 	print('Delete '+'../Firmware/'+new_zip_name)

# Check if the destination directory exists
if not os.path.exists('../Firmware/'): 
    os.makedirs('../Firmware/') 

# Copy the files
# HEX
# print("Copy HEX file") 
try:
	shutil.copy2(source_file, destination_directory+new_file_name)
except:
	print('Cannot copy '+source_file +' to '+destination_directory+new_file_name)

# ZIP
# print("Copy ZIP file") 
try:
	shutil.copy2(source_file_2, destination_directory+new_zip_name)
except:
	print('Cannot copy '+source_file_2 +' to '+destination_directory+new_zip_name)
