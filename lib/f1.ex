defmodule F1 do
  @moduledoc """
  Documentation for F1.
  """

  alias Circuits.GPIO
  require Logger

  @led_pin_1 20
  @led_pin_2 21
  @led_pin_3 22
  @led_pin_4 23
  @led_pin_5 24

  def start() do
    Logger.info("[ ==== F1 Start  ==== ]")
    {:ok, led_1_gpio} = GPIO.open(@led_pin_1, :output)
    {:ok, led_2_gpio} = GPIO.open(@led_pin_2, :output)
    {:ok, led_3_gpio} = GPIO.open(@led_pin_3, :output)
    {:ok, led_4_gpio} = GPIO.open(@led_pin_4, :output)
    {:ok, led_5_gpio} = GPIO.open(@led_pin_5, :output)
    lus(led_1_gpio, led_2_gpio, led_3_gpio, led_4_gpio, led_5_gpio)
  end

  def lus(led_1_gpio, led_2_gpio, led_3_gpio, led_4_gpio, led_5_gpio) do
    Logger.info("[ ==== F1 loop  ==== ]")

    GPIO.write(led_1_gpio, 1)
    Process.sleep(1_000)

    GPIO.write(led_2_gpio, 1)
    Process.sleep(1_000)

    GPIO.write(led_3_gpio, 1)
    Process.sleep(1_000)

    GPIO.write(led_4_gpio, 1)
    Process.sleep(1_000)

    GPIO.write(led_5_gpio, 1)

    willekeurige_tijd = Enum.random(1_000..3_000)
    Process.sleep(willekeurige_tijd)
    GPIO.write(led_1_gpio, 0)
    GPIO.write(led_2_gpio, 0)
    GPIO.write(led_3_gpio, 0)
    GPIO.write(led_4_gpio, 0)
    GPIO.write(led_5_gpio, 0)

    Process.sleep(5_000)

    lus(led_1_gpio, led_2_gpio, led_3_gpio, led_4_gpio, led_5_gpio)
  end
end
