#!/bin/sh

calcSha256 ()
{
	local sha256temp=$(eval "echo -n '"$1"' | sha256sum | head -c 64")
	echo ${sha256temp}
}

getApPass()
{
	# TODO! : apPwPrefixStr and apPwPostfixStr should be get from the configuration.
	# KEEP! : Recommend len(apPwPrefixStr) + len(apPwPostfixStr) + 6 < 64
	local apPwPrefixStr="Rj45Less-"
	local apPwPostfixStr="09azAZHeyyPleaseFillHereWithRandom09azAZ"
	local pwTestMid="$1"

	shaResult=$(calcSha256 "$apPwPrefixStr$pwTestMid$apPwPostfixStr")
	mabase62Result=$(mabase62 "$shaResult")

	echo "$mabase62Result"
}

getRootPass()
{
	# TODO! : rootPwPrefixStr and rootPwPostfixStr should be get from the configuration.
	# KEEP! : Recommend len(rootPwPrefixStr) + len(rootPwPostfixStr) + 6 < 64
	local rootPwPrefixStr="Rj45Less-"
	local rootPwPostfixStr="AhoyAhoyAhoyAhoyPekoraPekora"
	local pwTestMid="$1"

	shaResult=$(calcSha256 "$rootPwPrefixStr$pwTestMid$rootPwPostfixStr")
	mabase62Result=$(mabase62 "$shaResult")

	echo "$mabase62Result"
}

mid="$1"

echo "Mesh Id : $mid"
akey=$(getApPass "$mid")
rkey=$(getRootPass "$mid")
echo "WiFi Password : $akey"
echo "root Password : $rkey"