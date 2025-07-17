"use client"

import { useEffect, useState } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Badge } from "@/components/ui/badge"
import { Button } from "@/components/ui/button"
import { motion } from "framer-motion"

interface HotStock {
  symbol: string
  name: string
  price: number
  change: number
  changePercent: number
  logo: string
}

const tabs = ["All", "Stocks", "Crypto", "Forex", "Bonds"]

export function HotList() {
  const [activeTab, setActiveTab] = useState("All")
  const [hotStocks, setHotStocks] = useState<HotStock[]>([])

  useEffect(() => {
    // Mock data - replace with actual API call
    const mockStocks: HotStock[] = [
      { symbol: "TSLA", name: "TaskUS, Inc", price: 22.5, change: 47.42, changePercent: 47.42, logo: "T" },
      { symbol: "GROW", name: "Grow Generation", price: 10.33, change: 28.01, changePercent: 28.01, logo: "G" },
      { symbol: "SKY", name: "SkyWater", price: 11.37, change: 26.32, changePercent: 26.32, logo: "S" },
      { symbol: "LCO", name: "Light Crude Oil", price: 88.45, change: 20.19, changePercent: 20.19, logo: "L" },
      { symbol: "DOGE", name: "DogeCoin", price: 0.0899, change: 14.82, changePercent: 14.82, logo: "D" },
      { symbol: "XLM", name: "Stellar", price: 0.093766, change: 10.04, changePercent: 10.04, logo: "X" },
      { symbol: "NGEN", name: "NeoGenomics, Inc", price: 8.9, change: 4.23, changePercent: 4.23, logo: "N" },
      { symbol: "GRPN", name: "Groupon, Inc", price: 8.97, change: 3.73, changePercent: 3.73, logo: "G" },
      { symbol: "LYFT", name: "Lyft, Inc", price: 10.9, change: 2.32, changePercent: 2.32, logo: "L" },
    ]
    setHotStocks(mockStocks)
  }, [])

  return (
    <div className="space-y-6">
      {/* Hot List Card */}
      <Card className="border-gray-200">
        <CardHeader>
          <CardTitle className="text-lg font-semibold text-gray-900">Hot List</CardTitle>
          <div className="flex gap-1 mt-3">
            {tabs.map((tab) => (
              <Button
                key={tab}
                variant={activeTab === tab ? "default" : "ghost"}
                size="sm"
                onClick={() => setActiveTab(tab)}
                className={`text-xs px-3 py-1 h-7 ${
                  activeTab === tab
                    ? "bg-gray-900 text-white hover:bg-gray-800"
                    : "text-gray-600 hover:text-gray-900 hover:bg-gray-100"
                }`}
              >
                {tab}
              </Button>
            ))}
          </div>
        </CardHeader>
        <CardContent className="space-y-3">
          <div className="grid grid-cols-3 gap-2 text-xs text-gray-500 pb-2 border-b border-gray-100">
            <div>Symbol</div>
            <div className="text-right">Price</div>
            <div className="text-right">Gain</div>
          </div>
          {hotStocks.map((stock, index) => (
            <motion.div
              key={stock.symbol}
              initial={{ opacity: 0, y: 10 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ delay: index * 0.05 }}
              className="grid grid-cols-3 gap-2 items-center py-2 hover:bg-gray-50 rounded-lg px-2 -mx-2 cursor-pointer"
            >
              <div className="flex items-center gap-2">
                <div
                  className={`w-6 h-6 rounded-full flex items-center justify-center text-xs font-medium text-white ${
                    index % 5 === 0
                      ? "bg-red-500"
                      : index % 5 === 1
                        ? "bg-blue-500"
                        : index % 5 === 2
                          ? "bg-green-500"
                          : index % 5 === 3
                            ? "bg-gray-800"
                            : "bg-yellow-500"
                  }`}
                >
                  {stock.logo}
                </div>
                <div>
                  <div className="font-medium text-gray-900 text-sm">{stock.symbol}</div>
                  <div className="text-xs text-gray-500 truncate">{stock.name}</div>
                </div>
              </div>
              <div className="text-right">
                <div className="font-medium text-gray-900 text-sm">
                  {stock.price < 1 ? stock.price.toFixed(4) : stock.price.toFixed(2)}
                </div>
                <div className="text-xs text-gray-500">USD</div>
              </div>
              <div className="text-right">
                <Badge
                  className={`text-xs font-medium ${
                    stock.change >= 0
                      ? "bg-green-50 text-green-700 border-green-200"
                      : "bg-red-50 text-red-700 border-red-200"
                  }`}
                >
                  +{stock.changePercent.toFixed(2)}
                </Badge>
              </div>
            </motion.div>
          ))}
        </CardContent>
      </Card>

      {/* Swap Widget */}
      <Card className="border-gray-200 bg-gray-900 text-white">
        <CardHeader>
          <CardTitle className="text-lg font-semibold">Swap</CardTitle>
        </CardHeader>
        <CardContent className="space-y-4">
          <div>
            <div className="text-sm text-gray-400 mb-2">You Pay</div>
            <div className="flex items-center justify-between p-3 bg-gray-800 rounded-lg">
              <div className="text-2xl font-semibold">0</div>
              <div className="text-right">
                <div className="font-medium">ETH</div>
                <div className="text-xs text-gray-400">Balance: 0 ETH</div>
              </div>
            </div>
          </div>

          <div>
            <div className="text-sm text-gray-400 mb-2">You Get</div>
            <div className="flex items-center justify-between p-3 bg-gray-800 rounded-lg">
              <div className="text-2xl font-semibold">0</div>
              <div className="text-right">
                <div className="font-medium">BNB</div>
                <div className="text-xs text-gray-400">Balance: 0 BNB</div>
              </div>
            </div>
          </div>

          <div className="flex gap-2 text-xs">
            {["25%", "50%", "75%", "100%"].map((percent) => (
              <Button
                key={percent}
                variant="outline"
                size="sm"
                className="flex-1 h-8 bg-gray-800 border-gray-700 text-gray-300 hover:bg-gray-700"
              >
                {percent}
              </Button>
            ))}
          </div>

          <Button className="w-full bg-white text-gray-900 hover:bg-gray-100 font-medium">Swap</Button>
        </CardContent>
      </Card>
    </div>
  )
}
