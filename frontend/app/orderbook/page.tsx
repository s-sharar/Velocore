"use client"

import { useEffect, useState } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Badge } from "@/components/ui/badge"
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "@/components/ui/select"
import { BarChart3, TrendingDown, TrendingUp } from "lucide-react"
import { Progress } from "@/components/ui/progress"
import { motion, AnimatePresence } from "framer-motion"

interface OrderLevel {
  price: number
  quantity: number
  orders: number
}

interface OrderBookData {
  bids: OrderLevel[]
  asks: OrderLevel[]
  spread: number
  best_bid: number
  best_ask: number
}

export default function OrderBookPage() {
  const [orderBook, setOrderBook] = useState<OrderBookData | null>(null)
  const [levels, setLevels] = useState("10")
  const [loading, setLoading] = useState(true)

  useEffect(() => {
    const fetchOrderBook = async () => {
      try {
        const response = await fetch(`http://localhost:18080/orderbook?levels=${levels}`)
        if (response.ok) {
          const data = await response.json()
          setOrderBook(data.orderbook)
        }
      } catch (error) {
        console.error("Failed to fetch orderbook:", error)
      } finally {
        setLoading(false)
      }
    }

    fetchOrderBook()
    const interval = setInterval(fetchOrderBook, 2000)
    return () => clearInterval(interval)
  }, [levels])

  const getMaxQuantity = () => {
    if (!orderBook) return 0
    const allQuantities = [...orderBook.bids.map((b) => b.quantity), ...orderBook.asks.map((a) => a.quantity)]
    return Math.max(...allQuantities, 1)
  }

  return (
    <div className="space-y-6">
      {/* Header */}
      <motion.div initial={{ opacity: 0, y: -20 }} animate={{ opacity: 1, y: 0 }}>
        <div className="flex items-center justify-between">
          <div>
            <h1 className="text-2xl font-semibold text-gray-900">Order Book</h1>
            <p className="text-gray-600 mt-1">Real-time depth and liquidity visualization</p>
          </div>
          <div className="flex items-center gap-4">
            <Select value={levels} onValueChange={setLevels}>
              <SelectTrigger className="w-32 border-gray-200 focus:border-blue-300">
                <SelectValue />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="5">5 Levels</SelectItem>
                <SelectItem value="10">10 Levels</SelectItem>
                <SelectItem value="20">20 Levels</SelectItem>
              </SelectContent>
            </Select>
            <Badge className="bg-green-50 text-green-700 border-green-200 px-4 py-2">Live Data</Badge>
          </div>
        </div>
      </motion.div>

      {orderBook && (
        <motion.div
          initial={{ opacity: 0, y: 20 }}
          animate={{ opacity: 1, y: 0 }}
          className="grid gap-6 md:grid-cols-3 mb-8"
        >
          <Card className="border-gray-200">
            <CardHeader className="pb-2">
              <CardTitle className="text-sm text-gray-600">Best Bid</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="text-2xl font-semibold text-green-600">${orderBook.best_bid?.toFixed(2) || "0.00"}</div>
            </CardContent>
          </Card>

          <Card className="border-gray-200">
            <CardHeader className="pb-2">
              <CardTitle className="text-sm text-gray-600">Spread</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="text-2xl font-semibold text-blue-600">${orderBook.spread?.toFixed(2) || "0.00"}</div>
            </CardContent>
          </Card>

          <Card className="border-gray-200">
            <CardHeader className="pb-2">
              <CardTitle className="text-sm text-gray-600">Best Ask</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="text-2xl font-semibold text-red-600">${orderBook.best_ask?.toFixed(2) || "0.00"}</div>
            </CardContent>
          </Card>
        </motion.div>
      )}

      <div className="grid gap-6 lg:grid-cols-2 mb-8">
        <motion.div initial={{ opacity: 0, x: -20 }} animate={{ opacity: 1, x: 0 }}>
          <Card className="border-gray-200 h-[500px]">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-green-600">
                <TrendingUp className="h-5 w-5" />
                Bids (Buy Orders)
              </CardTitle>
            </CardHeader>
            <CardContent className="h-[400px] overflow-y-auto">
              {loading ? (
                <div className="space-y-2">
                  {[...Array(Number.parseInt(levels))].map((_, i) => (
                    <div key={i} className="h-12 bg-gray-100 animate-pulse rounded" />
                  ))}
                </div>
              ) : !orderBook?.bids?.length ? (
                <div className="text-center text-gray-500 py-8">No bid orders</div>
              ) : (
                <div className="space-y-1">
                  <div className="grid grid-cols-4 gap-2 text-xs font-medium text-gray-500 mb-2">
                    <div>Price</div>
                    <div>Size</div>
                    <div>Orders</div>
                    <div>Depth</div>
                  </div>
                  <AnimatePresence>
                    {orderBook.bids.map((bid, index) => (
                      <motion.div
                        key={bid.price}
                        initial={{ opacity: 0, y: 10 }}
                        animate={{ opacity: 1, y: 0 }}
                        exit={{ opacity: 0, y: -10 }}
                        transition={{ delay: index * 0.03 }}
                        className="relative"
                      >
                        <div
                          className="absolute inset-0 bg-green-50 rounded"
                          style={{ width: `${(bid.quantity / getMaxQuantity()) * 100}%` }}
                        />
                        <div className="relative grid grid-cols-4 gap-2 p-2 text-sm hover:bg-gray-50 rounded transition-colors">
                          <div className="font-mono text-green-600">${bid.price.toFixed(2)}</div>
                          <div className="font-mono text-gray-700">{bid.quantity.toLocaleString()}</div>
                          <div className="text-gray-500">{bid.orders}</div>
                          <div className="w-full">
                            <Progress value={(bid.quantity / getMaxQuantity()) * 100} className="h-2" />
                          </div>
                        </div>
                      </motion.div>
                    ))}
                  </AnimatePresence>
                </div>
              )}
            </CardContent>
          </Card>
        </motion.div>

        <motion.div initial={{ opacity: 0, x: 20 }} animate={{ opacity: 1, x: 0 }}>
          <Card className="border-gray-200 h-[500px]">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-red-600">
                <TrendingDown className="h-5 w-5" />
                Asks (Sell Orders)
              </CardTitle>
            </CardHeader>
            <CardContent className="h-[400px] overflow-y-auto">
              {loading ? (
                <div className="space-y-2">
                  {[...Array(Number.parseInt(levels))].map((_, i) => (
                    <div key={i} className="h-12 bg-gray-100 animate-pulse rounded" />
                  ))}
                </div>
              ) : !orderBook?.asks?.length ? (
                <div className="text-center text-gray-500 py-8">No ask orders</div>
              ) : (
                <div className="space-y-1">
                  <div className="grid grid-cols-4 gap-2 text-xs font-medium text-gray-500 mb-2">
                    <div>Price</div>
                    <div>Size</div>
                    <div>Orders</div>
                    <div>Depth</div>
                  </div>
                  <AnimatePresence>
                    {orderBook.asks.map((ask, index) => (
                      <motion.div
                        key={ask.price}
                        initial={{ opacity: 0, y: 10 }}
                        animate={{ opacity: 1, y: 0 }}
                        exit={{ opacity: 0, y: -10 }}
                        transition={{ delay: index * 0.03 }}
                        className="relative"
                      >
                        <div
                          className="absolute inset-0 bg-red-50 rounded"
                          style={{ width: `${(ask.quantity / getMaxQuantity()) * 100}%` }}
                        />
                        <div className="relative grid grid-cols-4 gap-2 p-2 text-sm hover:bg-gray-50 rounded transition-colors">
                          <div className="font-mono text-red-600">${ask.price.toFixed(2)}</div>
                          <div className="font-mono text-gray-700">{ask.quantity.toLocaleString()}</div>
                          <div className="text-gray-500">{ask.orders}</div>
                          <div className="w-full">
                            <Progress value={(ask.quantity / getMaxQuantity()) * 100} className="h-2" />
                          </div>
                        </div>
                      </motion.div>
                    ))}
                  </AnimatePresence>
                </div>
              )}
            </CardContent>
          </Card>
        </motion.div>
      </div>

      <motion.div initial={{ opacity: 0, y: 20 }} animate={{ opacity: 1, y: 0 }} transition={{ delay: 0.2 }}>
        <Card className="border-gray-200">
          <CardHeader>
            <CardTitle className="flex items-center gap-2 text-gray-900">
              <BarChart3 className="h-5 w-5 text-blue-600" />
              Order Book Depth Visualization
            </CardTitle>
          </CardHeader>
          <CardContent>
            <div className="h-64 flex items-end justify-center gap-1 p-4">
              {orderBook?.bids
                ?.slice()
                .reverse()
                .map((bid, index) => (
                  <motion.div
                    key={`bid-viz-${bid.price}`}
                    initial={{ opacity: 0, y: 50 }}
                    animate={{ opacity: 1, y: 0 }}
                    transition={{ delay: index * 0.02 }}
                    className="flex flex-col items-center"
                  >
                    <div
                      className="bg-green-500 w-8 min-h-[4px] rounded-t"
                      style={{ height: `${(bid.quantity / getMaxQuantity()) * 200}px` }}
                    />
                    <div className="text-xs text-green-600 mt-1 transform -rotate-45 origin-left">
                      ${bid.price.toFixed(2)}
                    </div>
                  </motion.div>
                ))}

              <div className="w-4 h-full flex items-center justify-center">
                <div className="h-full w-[2px] bg-gray-300" />
              </div>

              {orderBook?.asks?.map((ask, index) => (
                <motion.div
                  key={`ask-viz-${ask.price}`}
                  initial={{ opacity: 0, y: 50 }}
                  animate={{ opacity: 1, y: 0 }}
                  transition={{ delay: index * 0.02 }}
                  className="flex flex-col items-center"
                >
                  <div
                    className="bg-red-500 w-8 min-h-[4px] rounded-t"
                    style={{ height: `${(ask.quantity / getMaxQuantity()) * 200}px` }}
                  />
                  <div className="text-xs text-red-600 mt-1 transform -rotate-45 origin-left">
                    ${ask.price.toFixed(2)}
                  </div>
                </motion.div>
              ))}
            </div>
          </CardContent>
        </Card>
      </motion.div>
    </div>
  )
}
