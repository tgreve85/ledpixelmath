from ledpixelmath import Pixel

pixel = Pixel(5);

print(pixel.getIndex());
print(pixel.fillRgb([3, 4, 5]));
print(pixel.trigger());
print(pixel.trigger());
print(pixel.trigger());
print(pixel.trigger());
print(pixel.getFadeComplete());
print(pixel.trigger());
print(pixel.trigger());
print(pixel.getFadeComplete());

print(pixel.fadeToRgb([0, 0, 0]));
print(pixel.trigger());
print(pixel.trigger());
print(pixel.trigger());
print(pixel.trigger());
print(pixel.getFadeComplete());
print(pixel.trigger());
print(pixel.trigger());
print(pixel.getFadeComplete());
