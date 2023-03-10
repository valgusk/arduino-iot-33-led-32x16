#! /usr/bin/env ruby
require 'socket'

DIST = 5.0

def get_direction
  [rand(32), rand(16)]
end

begin
  client = nil
  client = TCPSocket.new('10.0.0.44', 80)

  position = [0, 0]
  direction = get_direction

  idx = 0

  loop do
    if position == direction
      direction = get_direction
    else
      xdiff = (direction[0] - position[0])
      ydiff = (direction[1] - position[1])
      max = [xdiff.abs, ydiff.abs].max

      position[0] += (xdiff.to_f / max).round
      position[1] += (ydiff.to_f / max).round
    end

    pixels = 16.times.flat_map do |row|
      32.times.map do |column|
        distance = Math.sqrt(((column - position[0]) ** 2 + (row - position[1]) ** 2).to_f)

        r = (DIST - [DIST, distance].min) / DIST
        g = (DIST - [DIST, (DIST - distance).abs].min) / DIST
        b = (DIST - [DIST, (DIST * 2 - distance).abs].min) / DIST

        r,g,b = [r, g, b].map { |float_val| (float_val * 15).round }

        if [r, g, b].all?(&:zero?)
          r = ((1.0 - ((distance.to_f - 2) / 35)) * 2).round
          b = ((1.0 - (distance.to_f / 36)) * 4).round
          g = ((1.0 - ((distance.to_f + 2) / 38)) * 4).round
        end

        [r, g, b]
      end
    end

    # pixels = 16.times.flat_map do |row|
    #   32.times.map do |column|
    #     # if row == idx % 16
    #       [2] * 3
    #     # else
    #     #   [0, 0, 0]
    #     # end
    #   end
    # end

    idx += 1

    data = pixels.each_slice(2).flat_map do |(pixela, pixelb)|
      [
        ((pixela[0] << 4) | pixela[1]),
        ((pixela[2] << 4) | pixelb[0]),
        ((pixelb[1] << 4) | pixelb[2]),
      ]
    end

    puts "STEP"

    string = data.map { |i| i.chr }.join

    client.write(string)
    puts client.gets(1.chr).inspect

    # sleep(0.2)
  end
ensure
  client&.close
end