
'''
A very short and sweet script to generate billboards for the names of the players.
'''

from PIL import Image, ImageDraw, ImageFont, ImageColor

def create_image(size, bgColor, message, font, fontColor):
    W, H = size
    image = Image.new('RGB', size, bgColor)
    draw = ImageDraw.Draw(image)
    _, _, w, h = draw.textbbox((0, 0), message, font=font)
    draw.text(((W-w)/2, 3), message, font=font, fill=fontColor)
    return image

names = []
names.append("Zeus")
names.append("Poseidon")
names.append("Ares")
names.append("Aphrodite")
names.append("Hera")
names.append("Demeter")
names.append("Athena")
names.append("Apollo")
names.append("Artemis")
names.append("Hephaestus")
names.append("Hermes")
names.append("Dionysus")

for name in names:
    myFont = ImageFont.truetype('JockeyOne.ttf', 24)
    myMessage = name
    myImage = create_image((120, 40), 'black', myMessage, myFont, 'white')
    myImage.save(f'{name}.png', "PNG")
