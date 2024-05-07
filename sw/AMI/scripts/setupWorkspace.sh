#!/bin/bash

################################################################################
#                                                                              #
# Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.        #
# SPDX-License-Identifier: MIT                                                 #
#                                                                              #
################################################################################

################################################################################
###                                 Variables                                ###
################################################################################
# Placeholder for identifying submodule or standalone git repository
what_am_i=""
# Placeholder for .git path
git_dir=""
# Placeholder for top level of repository
top_dir=""

### Artifactory file links ###
common_url_part="https://xcoartifactory.xilinx.com:443/artifactory/aps-software-dev/tools/uncrustify"
uncrustify_cfg="$common_url_part/.uncrustify_linux.cfg"
function_header_txt="$common_url_part/function_header.txt"
pre_commit="$common_url_part/pre-commit"
commit_msg="$common_url_part/commit-msg"
links=("$uncrustify_cfg" "$function_header_txt" "$pre_commit" "$commit_msg")

### AMI submodule paths ###
script_name="setupWorkspace.sh"
fal_path="driver/fal"
fal_script_path="$fal_path/$script_name"
gcq_path="driver/gcq-driver"
gcq_script_path="$gcq_path/$script_name"
submodule_paths=("$fal_script_path") #"$gcq_script_path"

### Jira paths ###
home_path=$(var=$(echo ~/) && var=${var::-1} && echo $var)
pat_dir="$home_path/.jira_pats"
pat_file="$pat_dir/pat"
task_file="$pat_dir/tasks"

base_url="https://jira.xilinx.com"

today=$(date -I)

################################################################################
###                                 Functions                                ###
################################################################################

SETUP_WORKSPACE_set_top_dir() {
	SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
	cd "$SCRIPT_DIR"
	top_dir=$(git rev-parse --show-toplevel)
}

SETUP_WORKSPACE_what_am_i() {
	cd $top_dir

	if [ -f ".git" ]; then
		# A submodule's .git is a file
		what_am_i="submodule"
	elif [ -d ".git" ]; then
		# A repositories .git is a directory
		what_am_i="repository"
	else
		echo "Failed to determine whether submodule or repository."
		exit 1
	fi
}

SETUP_WORKSPACE_set_git_dir() {
	cd $top_dir

	# Get .git directory location for this repository
	if [ "$what_am_i" = "submodule" ]; then
		git_dir=$(cat .git | cut -d ' ' -f2)
	elif [ "$what_am_i" = "repository" ]; then
		SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
		cd "$SCRIPT_DIR"
		git_dir=$(var=$(git rev-parse --show-toplevel) && echo "$var/.git")
	else
		echo "Failed to set the .git path."
		exit 1
	fi
}

SETUP_WORKSPACE_unc_check_installed() {
	check=$(command -v uncrustify)

	if [ $? -ne 0 ]; then
		echo "################################################################################"
		echo "#                                                                              #"
		echo "# Error: uncrustify not downloaded                                             #"
		echo "#                                                                              #"
		echo "# Please download uncrustify, see ==> https://github.com/uncrustify/uncrustify #"
		echo "#                                                                              #"
		echo "# Or follow these steps:                                                       #"
		echo "# git clone git@github.com:uncrustify/uncrustify.git ~/uncrustify_install      #"
		echo "# cd ~/uncrustify_install                                                      #"
		echo "# mkdir build                                                                  #"
		echo "# cd build                                                                     #"
		echo "# /* cmake 3.5 or more recent is needed for this */                            #"
		echo "# cmake -DCMAKE_BUILD_TYPE=Release ..                                          #"
		echo "# make                                                                         #"
		echo "# sudo make install                                                            #"
		echo "# /* If 'sudo make install' fails, copy the uncrustify executable in the build #"
		echo "# directory into your PATH. For example: sudo cp uncrustify /usr/local/bin/ */ #"
		echo "# /* clean uncrustify build */                                                 #"
		echo "# cd ..                                                                        #"
		echo "# rm -rf ~/uncrustify_install                                                  #"
		echo "#                                                                              #"
		echo "################################################################################"
		exit 1
	fi
}

SETUP_WORKSPACE_unc_check_version() {	
	current_ver="$(uncrustify -v | cut -d "-" -f2 | cut -d "-" -f1)"
	current_major="$(echo $current_ver | cut -d "." -f1)"
	current_minor="$(echo $current_ver | cut -d "." -f2)"
	current_patch="$(echo $current_ver | cut -d "." -f3)"

	# Minimum required version: Uncrustify-0.78.1-52-83bfd3e0a
	required_major="0"
	required_minor="78"
	required_patch="1"

	if [ $current_major -lt $required_major ]; then
		return 1
	elif [ $current_minor -lt $required_minor ]; then
		return 1
	elif [ $current_patch -lt $required_patch ]; then
		return 1
	else
		return 0
	fi
}

SETUP_WORKSPACE_unc_download() {
	cd $git_dir

	filename=$(echo $1 | rev | cut -d '/' -f1 | rev)
	# Assuming file is out of date, deleting to re-download latest version
	if [ -f "./hooks/$filename" ]; then
		rm ./hooks/$filename
	fi

	echo -n "Downloading to $(pwd)/hooks/ ==> $filename"
	curl -s -o "./hooks/$filename" $1
	if [ $? -eq 0 ]; then
		echo " - Success"
		chmod a+x ./hooks/$filename
	else
		echo " - Fail"
		echo "Check that the artifactory links are up to date."
		exit 1
	fi
}

SETUP_WORKSPACE_sub_setup() {
	# para1: submodule setupWorkspace.sh path from AMC clone top directory
	cd $top_dir
	./$1
}

SETUP_WORKSPACE_pat_dir() {
	if [ ! -d $pat_dir ]; then
		echo "==> Creating pat dir"
		mkdir $pat_dir
	fi
}

SETUP_WORKSPACE_pat_file() {
	if [ -f $pat_file ]; then
		pat_expiry_date=$(var=$(cat $pat_file) && var=${var#*expiringAt} && var=${var%rawToken*} && echo $var | cut -d '"' -f3 | cut -d 'T' -f1)
		if [[ "$today" > "$pat_expiry_date" ]] || [ "$today" = "$pat_expiry_date" ]; then
			echo "The current Jira PAT is not in date."
			mv $pat_file $pat_dir/tmp
			echo "==> Jira PAT file $pat_dir/pat moved to $pat_dir/tmp."
		fi
	fi
}

SETUP_WORKSPACE_pat() {
	# Loop until the request is successful
	while :
	do
		SETUP_WORKSPACE_pat_request
		SETUP_WORKSPACE_pat_checks
		if [ $? -eq 0 ]; then
			break
		fi
	done
}

SETUP_WORKSPACE_pat_request() {
	read -p 'Token name...........: ' token_name
	read -p 'Expires (max 90 days): ' exp_date

	read -p 'Jira Username........: ' username
	read -sp 'Jira Password........: ' password
	echo
	
	data="{\"name\": \"$token_name\",\"expirationDuration\": $exp_date}"

	curl \
		-s \
		-X POST \
		"$base_url/rest/pat/latest/tokens" \
		-H "Content-Type: application/json" \
		-d "$data" \
		-u "$username:$password" \
		-o $pat_file \
		> /dev/null
}

SETUP_WORKSPACE_pat_checks() {
	# Check PAT file does not include..
	declare -a auth_output=("Authentication Failure"
				"Unauthorized"
				"error while loading this page")
	for check in "${auth_output[@]}"; do
	if grep -Fqi "$check" $pat_file; then
		echo "Check that failed: $check"
		echo "The PAT create request failed to authenticate."
		return 1
	fi
	done

	# Check PAT file includes..
	declare -a pat_output=("id"
				"name"
				"createdAt"
				"expiringAt"
				"createdAt"
				"rawToken")
	for check in "${pat_output[@]}"; do
	if ! grep -Fqi "$check" $pat_file; then
		echo "Check that failed: $check"
		echo "The PAT request return failed to contain required data."
		return 1
	fi
	done
}

################################################################################
###                           Script Starting Point                          ###
################################################################################

echo "=== AMI ==="
 
SETUP_WORKSPACE_unc_check_installed
SETUP_WORKSPACE_unc_check_version
if [ $? -eq 0 ]; then
	echo "Valid uncrustify version."
else
	echo "Error: invalid uncrustify version, please update the tool."
	exit 1
fi

SETUP_WORKSPACE_set_top_dir
SETUP_WORKSPACE_what_am_i
SETUP_WORKSPACE_set_git_dir

for link in "${links[@]}"
do
	SETUP_WORKSPACE_unc_download "$link"
done

for path in "${submodule_paths[@]}"
do
	SETUP_WORKSPACE_sub_setup "$path"
done

# PAT setup
SETUP_WORKSPACE_pat_dir
SETUP_WORKSPACE_pat_file
# PAT setup, if PAT file does not not exist then a new one is needed.
if [ ! -f $pat_file ]; then
	SETUP_WORKSPACE_pat
	echo "==> New Jira PAT file created: $pat_file"
fi
