echo ""
echo "Compiling source files ..."

rm -rf build > /dev/null
mkdir build

gcc -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./devicemanagementclient.c  -o devicemanagementclient.o
gcc -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./gatewayclient.c  -o gatewayclient.o
gcc -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./iotfclient.c  -o iotfclient.o
gcc  -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./lib/MQTTClient.c -o mqttclient.o
gcc  -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./lib/MQTTLinux.c -o mqttlinux.o
gcc  -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./lib/MQTTFormat.c -o mqttformat.o  
gcc  -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./lib/MQTTPacket.c -o mqttpacket.o
gcc  -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./lib/MQTTDeserializePublish.c -o mqttdespublish.o
gcc  -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./lib/MQTTConnectClient.c -o mqttconnectclient.o
gcc  -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./lib/MQTTSubscribeClient.c -o mqttsubscribeclient.o
gcc  -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./lib/MQTTSerializePublish.c -o mqttserpublish.o
gcc  -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./lib/MQTTConnectServer.c -o mqttconnectserver.o
gcc  -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./lib/MQTTSubscribeServer.c -o mqttsubscribeserver.o
gcc  -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./lib/MQTTUnsubscribeServer.c -o mqttunsubscribeserver.o
gcc  -Wall -fPIC -c -D_GNU_SOURCE -I./ -I./lib ./lib/MQTTUnsubscribeClient.c -o mqttunsubscribeclient.o
echo ""
echo "Linking libiotfdevmgt.so libiotf.so libwiotdevice.so"

gcc -shared -Wl,-soname,libiotfdevmgt.so -lm -ldl -lpthread -o ./build/libiotfdevmgt.so devicemanagementclient.o mqttclient.o mqttlinux.o mqttformat.o  mqttpacket.o mqttdespublish.o mqttconnectclient.o mqttsubscribeclient.o mqttserpublish.o mqttconnectserver.o mqttsubscribeserver.o mqttunsubscribeserver.o mqttunsubscribeclient.o
gcc -shared -Wl,-soname,libiotf.so -lm -ldl -lpthread -o ./build/libiotf.so gatewayclient.o mqttclient.o mqttlinux.o mqttformat.o  mqttpacket.o mqttdespublish.o mqttconnectclient.o mqttsubscribeclient.o mqttserpublish.o mqttconnectserver.o mqttsubscribeserver.o mqttunsubscribeserver.o mqttunsubscribeclient.o
gcc -shared -Wl,-soname,libwiotdevice.so -lm -ldl -lpthread -o ./build/libwiotdevice.so iotfclient.o mqttclient.o mqttlinux.o mqttformat.o  mqttpacket.o mqttdespublish.o mqttconnectclient.o mqttsubscribeclient.o mqttserpublish.o mqttconnectserver.o mqttsubscribeserver.o mqttunsubscribeserver.o mqttunsubscribeclient.o
echo ""
echo "Removing temporary files..."
rm devicemanagementclient.o iotfclient.o gatewayclient.o mqttclient.o mqttlinux.o mqttformat.o  mqttpacket.o mqttdespublish.o mqttconnectclient.o mqttsubscribeclient.o mqttserpublish.o mqttconnectserver.o mqttsubscribeserver.o mqttunsubscribeserver.o mqttunsubscribeclient.o
echo ""
echo "Build complete"
