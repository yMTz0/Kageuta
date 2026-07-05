from PIL import Image
img = Image.open('C:/Users/jps3d/Kageuta-Cpp/assets/icon.png')
sizes = [(256,256),(128,128),(64,64),(48,48),(32,32),(16,16)]
img.save('C:/Users/jps3d/Kageuta-Cpp/assets/icon.ico', format='ICO', sizes=sizes)
print('ICO created')
