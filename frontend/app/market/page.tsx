"use client"

import type React from "react"
import { useState, useEffect } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { Input } from "@/components/ui/input"
import { Label } from "@/components/ui/label"
import { Checkbox } from "@/components/ui/checkbox"
import { Badge } from "@/components/ui/badge"
import { useToast } from "@/hooks/use-toast"
import { TrendingUp, Plus, Wifi, Loader2 } from "lucide-react"
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table"
import { motion, AnimatePresence } from "framer-motion"

interface MarketTick {
  symbol: string
  type: string
  timestamp: number
  trade_price?: number
  trade_size?: number
  bid_price?: number
  ask_price?: number
  bid_size?: number
  ask_size?: number
  open?: number
  high?: number
  low?: number
  close?: number
  volume?: number
}

interface SubscriptionForm {
  symbol: string
  trades: boolean
  quotes: boolean
  bars: boolean
}

export default function MarketDataPage() {
  const [formData, setFormData] = useState<SubscriptionForm>({
    symbol: "AAPL",
    trades: true,
    quotes: true,
    bars: false,
  })
  const [marketData, setMarketData] = useState<MarketTick[]>([])
  const [subscribedSymbols, setSubscribedSymbols] = useState<string[]>([])
  const [connected, setConnected] = useState(false)
  const [loading, setLoading] = useState(false)
  const { toast } = useToast()

  const fetchMarketData = async () => {
    try {
      const response = await fetch("http://localhost:18080/market/data")
      if (response.ok) {
        const data = await response.json()
        setMarketData(data.ticks || [])
      }
    } catch (error) {
      console.error("Failed to fetch market data:", error)
    }
  }

  const fetchStatus = async () => {
    try {
      const response = await fetch("http://localhost:18080/market/status")
      if (response.ok) {
        const data = await response.json()
        setConnected(data.connected)
        setSubscribedSymbols(data.subscribed_symbols || [])
      }
    } catch (error) {
      console.error("Failed to fetch status:", error)
      setConnected(false)
    }
  }

  useEffect(() => {
    fetchMarketData()
    fetchStatus()

    const dataInterval = setInterval(fetchMarketData, 2000)
    const statusInterval = setInterval(fetchStatus, 5000)

    return () => {
      clearInterval(dataInterval)
      clearInterval(statusInterval)
    }
  }, [])

  const handleSubscribe = async (e: React.FormEvent) => {
    e.preventDefault()
    setLoading(true)

    try {
      const response = await fetch("http://localhost:18080/market/subscribe", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(formData),
      })

      if (response.ok) {
        toast({
          title: "Subscription Successful",
          description: `Subscribed to ${formData.symbol} market data`,
        })

        setFormData({
          symbol: "AAPL",
          trades: true,
          quotes: true,
          bars: false,
        })

        setTimeout(fetchStatus, 1000)
      } else {
        const error = await response.text()
        toast({
          title: "Subscription Failed",
          description: error || "Failed to subscribe to market data",
          variant: "destructive",
        })
      }
    } catch (error) {
      toast({
        title: "Error",
        description: "Network error occurred",
        variant: "destructive",
      })
    } finally {
      setLoading(false)
    }
  }

  const formatTimestamp = (timestamp: number) => {
    return new Date(timestamp).toLocaleTimeString()
  }

  const getTypeColor = (type: string) => {
    switch (type) {
      case "TRADE":
        return "bg-green-50 text-green-700 border-green-200"
      case "QUOTE":
        return "bg-blue-50 text-blue-700 border-blue-200"
      case "BAR":
        return "bg-purple-50 text-purple-700 border-purple-200"
      default:
        return "bg-gray-50 text-gray-700 border-gray-200"
    }
  }

  return (
    <div className="space-y-6">
      {/* Header */}
      <motion.div initial={{ opacity: 0, y: -20 }} animate={{ opacity: 1, y: 0 }}>
        <div className="flex items-center justify-between">
          <div>
            <h1 className="text-2xl font-semibold text-gray-900">Market Data</h1>
            <p className="text-gray-600 mt-1">Subscribe to real-time market feeds</p>
          </div>
          <div className="flex items-center gap-4">
            <Badge
              className={
                connected ? "bg-green-50 text-green-700 border-green-200" : "bg-red-50 text-red-700 border-red-200"
              }
            >
              <Wifi className="h-3 w-3 mr-2" />
              {connected ? "Connected" : "Disconnected"}
            </Badge>
          </div>
        </div>
      </motion.div>

      <div className="grid gap-6 lg:grid-cols-3">
        <motion.div initial={{ opacity: 0, x: -20 }} animate={{ opacity: 1, x: 0 }}>
          <Card className="border-gray-200">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-gray-900">
                <Plus className="h-5 w-5 text-blue-600" />
                Subscribe to Market Data
              </CardTitle>
            </CardHeader>
            <CardContent>
              <form onSubmit={handleSubscribe} className="space-y-4">
                <div>
                  <Label htmlFor="symbol" className="text-gray-700">
                    Symbol
                  </Label>
                  <Input
                    id="symbol"
                    value={formData.symbol}
                    onChange={(e) => setFormData({ ...formData, symbol: e.target.value.toUpperCase() })}
                    placeholder="AAPL"
                    className="border-gray-200 focus:border-blue-300 focus:ring-2 focus:ring-blue-100"
                    required
                  />
                </div>

                <div className="space-y-3">
                  <Label className="text-gray-700">Data Types</Label>
                  <div className="space-y-2">
                    <div className="flex items-center space-x-2">
                      <Checkbox
                        id="trades"
                        checked={formData.trades}
                        onCheckedChange={(checked) => setFormData({ ...formData, trades: !!checked })}
                        className="border-gray-300 data-[state=checked]:bg-blue-600 data-[state=checked]:text-white"
                      />
                      <Label htmlFor="trades" className="text-gray-700">
                        Trades
                      </Label>
                    </div>
                    <div className="flex items-center space-x-2">
                      <Checkbox
                        id="quotes"
                        checked={formData.quotes}
                        onCheckedChange={(checked) => setFormData({ ...formData, quotes: !!checked })}
                        className="border-gray-300 data-[state=checked]:bg-blue-600 data-[state=checked]:text-white"
                      />
                      <Label htmlFor="quotes" className="text-gray-700">
                        Quotes
                      </Label>
                    </div>
                    <div className="flex items-center space-x-2">
                      <Checkbox
                        id="bars"
                        checked={formData.bars}
                        onCheckedChange={(checked) => setFormData({ ...formData, bars: !!checked })}
                        className="border-gray-300 data-[state=checked]:bg-blue-600 data-[state=checked]:text-white"
                      />
                      <Label htmlFor="bars" className="text-gray-700">
                        Bars (OHLCV)
                      </Label>
                    </div>
                  </div>
                </div>

                <Button
                  type="submit"
                  className="w-full bg-blue-600 hover:bg-blue-700 text-white font-medium py-2"
                  disabled={loading || !connected}
                >
                  {loading ? <Loader2 className="h-4 w-4 mr-2 animate-spin" /> : <Plus className="h-4 w-4 mr-2" />}
                  {loading ? "Subscribing..." : "Subscribe"}
                </Button>
              </form>
            </CardContent>
          </Card>
        </motion.div>

        <motion.div
          initial={{ opacity: 0, x: 20 }}
          animate={{ opacity: 1, x: 0 }}
          transition={{ delay: 0.2 }}
          className="lg:col-span-2"
        >
          <Card className="border-gray-200">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-gray-900">
                <TrendingUp className="h-5 w-5 text-blue-600" />
                Subscribed Symbols ({subscribedSymbols.length})
              </CardTitle>
            </CardHeader>
            <CardContent>
              {subscribedSymbols.length === 0 ? (
                <div className="text-center text-gray-500 py-12">
                  <TrendingUp className="h-12 w-12 mx-auto mb-4 opacity-50" />
                  <p>No active subscriptions</p>
                </div>
              ) : (
                <div className="grid gap-3 md:grid-cols-2">
                  {subscribedSymbols.map((symbol, index) => (
                    <motion.div
                      key={symbol}
                      initial={{ opacity: 0, y: 10 }}
                      animate={{ opacity: 1, y: 0 }}
                      transition={{ delay: index * 0.05 }}
                      className="flex justify-between items-center p-3 border border-gray-100 rounded-lg hover:border-gray-200 transition-all duration-200 hover:bg-gray-50"
                    >
                      <span className="font-medium text-gray-900">{symbol}</span>
                      <Badge className="bg-green-50 text-green-700 border-green-200">Active</Badge>
                    </motion.div>
                  ))}
                </div>
              )}
            </CardContent>
          </Card>
        </motion.div>
      </div>

      <motion.div initial={{ opacity: 0, y: 20 }} animate={{ opacity: 1, y: 0 }} transition={{ delay: 0.2 }}>
        <Card className="border-gray-200">
          <CardHeader>
            <CardTitle className="text-gray-900">Real-Time Market Data Feed</CardTitle>
          </CardHeader>
          <CardContent>
            {marketData.length === 0 ? (
              <div className="text-center text-gray-500 py-12">
                <Wifi className="h-12 w-12 mx-auto mb-4 opacity-50" />
                <p>No market data available. Subscribe to symbols above to see live data.</p>
              </div>
            ) : (
              <div className="overflow-x-auto">
                <Table>
                  <TableHeader>
                    <TableRow className="border-gray-200">
                      <TableHead className="text-gray-600">Symbol</TableHead>
                      <TableHead className="text-gray-600">Type</TableHead>
                      <TableHead className="text-gray-600">Price/Bid</TableHead>
                      <TableHead className="text-gray-600">Size/Ask</TableHead>
                      <TableHead className="text-gray-600">Additional</TableHead>
                      <TableHead className="text-gray-600">Time</TableHead>
                    </TableRow>
                  </TableHeader>
                  <TableBody>
                    <AnimatePresence>
                      {marketData
                        .slice(-20)
                        .reverse()
                        .map((tick, index) => (
                          <motion.tr
                            key={index}
                            initial={{ opacity: 0, y: 10 }}
                            animate={{ opacity: 1, y: 0 }}
                            exit={{ opacity: 0, y: -10 }}
                            transition={{ delay: index * 0.02 }}
                            className="border-gray-100 hover:bg-gray-50"
                          >
                            <TableCell className="font-medium text-gray-900">{tick.symbol}</TableCell>
                            <TableCell>
                              <Badge className={getTypeColor(tick.type)}>{tick.type}</Badge>
                            </TableCell>
                            <TableCell className="font-mono text-gray-700">
                              {tick.type === "TRADE" && `$${tick.trade_price?.toFixed(2)}`}
                              {tick.type === "QUOTE" && `$${tick.bid_price?.toFixed(2)}`}
                              {tick.type === "BAR" && `$${tick.close?.toFixed(2)}`}
                            </TableCell>
                            <TableCell className="text-gray-700">
                              {tick.type === "TRADE" && tick.trade_size?.toLocaleString()}
                              {tick.type === "QUOTE" && `$${tick.ask_price?.toFixed(2)}`}
                              {tick.type === "BAR" && tick.volume?.toLocaleString()}
                            </TableCell>
                            <TableCell className="text-gray-500 text-sm">
                              {tick.type === "QUOTE" && `${tick.bid_size} x ${tick.ask_size}`}
                              {tick.type === "BAR" && `H: $${tick.high?.toFixed(2)} L: $${tick.low?.toFixed(2)}`}
                            </TableCell>
                            <TableCell className="text-gray-500 text-sm">{formatTimestamp(tick.timestamp)}</TableCell>
                          </motion.tr>
                        ))}
                    </AnimatePresence>
                  </TableBody>
                </Table>
              </div>
            )}
          </CardContent>
        </Card>
      </motion.div>
    </div>
  )
}
