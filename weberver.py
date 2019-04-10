#!/usr/bin/env python
from __future__ import print_function
from flask import Flask, render_template, request
import color_wheel as cw
import argparse
import strobe_wheel

app = Flask(__name__)
LEDS = 4

@app.route('/',methods = ['POST', 'GET'])
def result():
    if request.method == 'POST':
        result = request.form
        write_program(result)
        return render_template("settings.html", result=result)
    else:
        return render_template("settings.html", result={
            'color':'rgb(255, 128, 0)',
            'brightness':'13',
            'program':'color_wheel',
        })

def write_program(ops):
    if ops['program'] == 'solid_color':
        brightness = int(ops['brightness']) / 255.0
        color = ops['color'].replace('rgb(','').replace(')','').replace(',','')
        color = map(int, color.split(' '))
        color = cw.scale(color, brightness)
        print('100 0 ' + ' '.join([' '.join(map(str, color))]*5))
        print('END')
    elif ops['program'] == 'color_wheel':
        for point in cw.color_wheel(128 * len(cw.PTS)):
            print(1, 0,
                ' '.join(
                    [' '.join(str(x) for x in cw.scale(point, int(ops['brightness']))) for i in range(LEDS)]
                )
            )
        print('END')
    elif ops['program'] == 'strobe':
        strobe_wheel.strobe_wheel(64, 1, int(ops['brightness']), LEDS)



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Create a color wheel animation")
    parser.add_argument("--leds", type=int, default=4, help="How many LEDs to generate channels for? (def 4)")
    parser.add_argument("--port", type=int, default=5000, help="Port to listen on")
    parser.add_argument("--host", default='127.0.0.1', help="Host to listen on")
    args = parser.parse_args()
    LEDS = args.leds
    app.run(host=args.host, port=args.port, debug=True)