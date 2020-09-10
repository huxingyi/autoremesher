"""Script for plotting the results of the 'suite' benchmark.
Invoke without parameters for usage hints.

:Author: Francesc Alted
:Date: 2010-06-01
"""
from __future__ import print_function

import matplotlib as mpl
from pylab import *

KB_ = 1024
MB_ = 1024*KB_
GB_ = 1024*MB_
NCHUNKS = 128    # keep in sync with bench.c

linewidth=2
#markers= ['+', ',', 'o', '.', 's', 'v', 'x', '>', '<', '^']
#markers= [ 'x', '+', 'o', 's', 'v', '^', '>', '<', ]
markers= [ 's', 'o', 'v', '^', '+', 'x', '>', '<', '.', ',' ]
markersize = 8

def get_values(filename):
    f = open(filename)
    values = {"memcpyw": [], "memcpyr": []}

    for line in f:
        if line.startswith('-->'):
            tmp = line.split('-->')[1]
            parts = tmp.split(', ')
            nthreads, size, elsize, sbits, codec, shuffle = parts[:6]
            safe = 'unsafe'
            if len(parts) > 6:
                safe = parts[6]
            nthreads, size, elsize, sbits = map(int, (nthreads, size, elsize, sbits))
            values["size"] = size * NCHUNKS / MB_
            values["elsize"] = elsize
            values["sbits"] = sbits
            values["codec"] = codec
            values["shuffle"] = shuffle
            values["safe"] = safe
            # New run for nthreads
            (ratios, speedsw, speedsr) = ([], [], [])
            # Add a new entry for (ratios, speedw, speedr)
            values[nthreads] = (ratios, speedsw, speedsr)
            #print("-->", nthreads, size, elsize, sbits)
        elif line.startswith('memcpy(write):'):
            tmp = line.split(',')[1]
            memcpyw = float(tmp.split(' ')[1])
            values["memcpyw"].append(memcpyw)
        elif line.startswith('memcpy(read):'):
            tmp = line.split(',')[1]
            memcpyr = float(tmp.split(' ')[1])
            values["memcpyr"].append(memcpyr)
        elif line.startswith('comp(write):'):
            tmp = line.split(',')[1]
            speedw = float(tmp.split(' ')[1])
            ratio = float(line.split(':')[-1])
            speedsw.append(speedw)
            ratios.append(ratio)
        elif line.startswith('decomp(read):'):
            tmp = line.split(',')[1]
            speedr = float(tmp.split(' ')[1])
            speedsr.append(speedr)
            if "OK" not in line:
                print("WARNING!  OK not found in decomp line!")

    f.close()
    return nthreads, values


def show_plot(plots, yaxis, legends, gtitle, xmax=None, ymax=None):
    xlabel('Compresssion ratio')
    ylabel('Speed (MB/s)')
    title(gtitle)
    xlim(0, xmax)
    ylim(0, ymax)
    grid(True)

#     legends = [f[f.find('-'):f.index('.out')] for f in filenames]
#     legends = [l.replace('-', ' ') for l in legends]
    #legend([p[0] for p in plots], legends, loc = "upper left")
    legend([p[0] for p in plots
            if not isinstance(p, mpl.lines.Line2D)],
           legends, loc = "best")


    #subplots_adjust(bottom=0.2, top=None, wspace=0.2, hspace=0.2)
    if outfile:
        print("Saving plot to:", outfile)
        savefig(outfile, dpi=64)
    else:
        show()

if __name__ == '__main__':

    from optparse import OptionParser

    usage = "usage: %prog [-r] [-o outfile] [-t title ] [-d|-c] filename"
    compress_title = 'Compression speed'
    decompress_title = 'Decompression speed'
    yaxis = 'No axis name'

    parser = OptionParser(usage=usage)
    parser.add_option('-o',
                      '--outfile',
                      dest='outfile',
                      help=('filename for output (many extensions '
                            'supported, e.g. .png, .jpg, .pdf)'))

    parser.add_option('-t',
                      '--title',
                      dest='title',
                      help='title of the plot',)

    parser.add_option('-l',
                      '--limit',
                      dest='limit',
                      help='expression to limit number of threads shown',)

    parser.add_option('-x',
                      '--xmax',
                      dest='xmax',
                      help='limit the x-axis',
                      default=None)

    parser.add_option('-y',
                      '--ymax',
                      dest='ymax',
                      help='limit the y-axis',
                      default=None)

    parser.add_option('-r', '--report', action='store_true',
                      dest='report',
                      help='generate file for reporting ',
                      default=False)

    parser.add_option('-d', '--decompress', action='store_true',
            dest='dspeed',
            help='plot decompression data',
            default=False)
    parser.add_option('-c', '--compress', action='store_true',
            dest='cspeed',
            help='plot compression data',
            default=False)

    (options, args) = parser.parse_args()
    if len(args) == 0:
        parser.error("No input arguments")
    elif len(args) > 1:
        parser.error("Too many input arguments")
    else:
        pass

    if options.report and options.outfile:
        parser.error("Can only select one of [-r, -o]")

    if options.dspeed and options.cspeed:
        parser.error("Can only select one of [-d, -c]")
    elif options.cspeed:
        options.dspeed = False
        plot_title = compress_title
    else: # either neither or dspeed
        options.dspeed = True
        plot_title = decompress_title

    filename = args[0]
    cspeed = options.cspeed
    dspeed = options.dspeed
    if options.outfile:
        outfile = options.outfile
    elif options.report:
        if cspeed:
            outfile = filename[:filename.rindex('.')] + '-compr.png'
        else:
            outfile = filename[:filename.rindex('.')] + '-decompr.png'
    else:
        outfile = None

    plots = []
    legends = []
    nthreads, values = get_values(filename)
    #print("Values:", values)

    if options.limit:
        thread_range = eval(options.limit)
    else:
        thread_range = range(1, nthreads+1)

    if options.title:
        plot_title = options.title
    else:
        plot_title += " (%(size).1f MB, %(elsize)d bytes, %(sbits)d bits), %(codec)s %(shuffle)s %(safe)s" % values

    gtitle = plot_title

    for nt in thread_range:
        #print("Values for %s threads --> %s" % (nt, values[nt]))
        (ratios, speedw, speedr) = values[nt]
        if cspeed:
            speed = speedw
        else:
            speed = speedr
        #plot_ = semilogx(ratios, speed, linewidth=2)
        plot_ = plot(ratios, speed, linewidth=2)
        plots.append(plot_)
        nmarker = nt
        if nt >= len(markers):
            nmarker = nt%len(markers)
        setp(plot_, marker=markers[nmarker], markersize=markersize,
             linewidth=linewidth)
        legends.append("%d threads" % nt)

    # Add memcpy lines
    if cspeed:
        mean = np.mean(values["memcpyw"])
        message = "memcpy (write to memory)"
    else:
        mean = np.mean(values["memcpyr"])
        message = "memcpy (read from memory)"
    plot_ = axhline(mean, linewidth=3, linestyle='-.', color='black')
    text(4.0, mean+400, message)
    plots.append(plot_)
    show_plot(plots, yaxis, legends, gtitle,
              xmax=int(options.xmax) if options.xmax else None,
              ymax=int(options.ymax) if options.ymax else None)
